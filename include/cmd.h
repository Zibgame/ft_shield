
#pragma once

#include <winsock2.h>
#include <windows.h>

typedef struct s_cmd
{
    HANDLE process;
    HANDLE stdin_write;
    HANDLE stdout_read;
}   t_cmd;

t_cmd cmd_init(void);
int cmd_start(t_cmd *cmd);
void cmd_stop(t_cmd *cmd);
int cmd_prepare(t_cmd *cmd);
int cmd_exec(t_cmd *cmd, const char *command);
int cmd_send_output(t_cmd *cmd, SOCKET slot);
