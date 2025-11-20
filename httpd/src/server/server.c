#define _POSIX_C_SOURCE 200112L
#include "server.h"

#include <err.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define LOGGER(FMT, ...) fprintf(stderr, FMT, ##__VA_ARGS__)

void respond(int client_fd, const char *buffer, ssize_t bytes)
{
    ssize_t total = 0;
    ssize_t sent = 0;
    // printf("%s this is buffer", buffer);
    while (total != bytes)
    {
        sent = send(client_fd, buffer + total, bytes - total, 0);
        if (sent == -1)
        {
            LOGGER("Send failed\n");
            return;
        }
        total += sent;
    }
}
int create_and_bind(const char *node, const char *service)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = NULL;
    if (getaddrinfo(node, service, &hints, &res) == -1)
    {
        LOGGER("create_and_bind: failed getaddrinfo\n");
        return -1;
    }

    int sockfd = -1;

    for (struct addrinfo *p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;

        int yes = -1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) != -1)
            break;
        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);
    return sockfd;
}
void communicate(int client_fd)
{
    ssize_t bytes = 0;
    char buffer[BUFFER_SIZE];
    while ((bytes = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        LOGGER("Received: %s", buffer);
        respond(client_fd, buffer, bytes);
    }
}

void start_server(int server_socket)
{
    if (listen(server_socket, SOMAXCONN) == -1)
    {
        return;
    }
    while (1)
    {
        int cfd = accept(server_socket, NULL, NULL);
        if (cfd != -1)
        {
            LOGGER("Client connected\n");
            communicate(cfd);
            close(cfd);
            LOGGER("Client disconnected\n");
        }
    }
}
