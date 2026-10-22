
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
        return INVALID_SOCKET;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        WSACleanup();
        return INVALID_SOCKET;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(4242);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return sock;
}
