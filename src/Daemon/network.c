/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   network.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42lehavre.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/22 17:00:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/10/22 17:00:00 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_shield.h"
#include "daemon.h"

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
