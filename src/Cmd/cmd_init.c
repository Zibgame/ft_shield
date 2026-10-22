
#include "ft_shield.h"

t_cmd cmd_init(void)
{
    t_cmd cmd;

    cmd.process = NULL;
    cmd.stdin_write = NULL;
    cmd.stdout_read = NULL;
    return (cmd);
}
