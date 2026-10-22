/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 17:00:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 17:00:00 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DAEMON_H
# define DAEMON_H

# include <winsock2.h>
# include "cmd.h"

# define MAX_CLIENTS 3

typedef struct s_client
{
    SOCKET  socket;
    int     authenticated;
    int     shell_mode;
    long    bytes_in;
    long    bytes_out;
    t_cmd   cmd;
}   t_client;

SOCKET      init_server(void);
void        client_reset(t_client *client, SOCKET socket);
void        client_accept(SOCKET server_fd, t_client clients[], fd_set *fd_list);
void        client_disconnect(t_client clients[], SOCKET current, fd_set *fd_list);
void        client_handle(t_client clients[], SOCKET current, const char *buffer);
const char  *help_message(void);
int         mode_daemon(void);

#endif
