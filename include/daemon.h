
#ifndef DAEMON_H
# define DAEMON_H

# include <winsock2.h>
# include "cmd.h"

# define MAX_CLIENTS 3

# define COLOR_RESET   "\x1b[0m"
# define COLOR_RED     "\x1b[31m"
# define COLOR_GREEN   "\x1b[32m"
# define COLOR_YELLOW  "\x1b[33m"
# define COLOR_CYAN    "\x1b[36m"

typedef struct s_client
{
    SOCKET  socket;
    int     authenticated;
    int     shell_mode;
    long    bytes_in;
    long    bytes_out;
    t_cmd   cmd;
}   t_client;

int         cli_send(t_client *client, SOCKET socket, const char *buf, int len);
int         cli_send_str(t_client *client, SOCKET socket, const char *buf);
SOCKET      init_server(void);
void        client_reset(t_client *client, SOCKET socket);
void        client_accept(SOCKET server_fd, t_client clients[], fd_set *fd_list);
void        client_disconnect(t_client clients[], SOCKET current, fd_set *fd_list);
void        client_handle(t_client clients[], SOCKET current, const char *buffer);
const char  *help_message(void);
int         mode_daemon(void);

#endif
