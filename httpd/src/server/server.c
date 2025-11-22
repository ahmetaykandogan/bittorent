#define _POSIX_C_SOURCE 200112L
#include "server.h"

#include <err.h>
#include <netdb.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../utils/parser/parser.h"
#include "../utils/time/time.h"

#define BUFFER_SIZE 1024
static char *create_respond(const char *buffer, struct response *resp,
                            char *out)
{
    parser(buffer, resp);
    char time_in[80];
    char *time = print_date(time_in);
    int length = 12;
    sprintf(out, "%s 200 OK\r\n%s\r\nContent-Length: %d\r\nConnection: close\r\n", resp->http_type, time,length);
    return out;
}
static void respond(int client_fd, const char *buffer, ssize_t bytes,
                    struct response *resp)
{
    ssize_t total = 0;
    ssize_t sent = 0;
    char out[200];
    char *sending = create_respond(buffer, resp, out);
    sent = send(client_fd, sending, strlen(sending), 0);
    while (total != bytes && total == 5454)
    {
        sent = send(client_fd, buffer + total, bytes - total, 0);
        if (sent == -1)
        {
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
static void communicate(int client_fd, struct response *resp)
{
    ssize_t bytes = 0;
    char buffer[BUFFER_SIZE] = { 0 };
    char *total = calloc(1024, 1);
    int actual_size = 1024;
    int total_bytes = 0;
    int counter = 0;
    while ((bytes = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        total_bytes += bytes;
        if (total_bytes > actual_size)
        {
            actual_size += 1024;
            total = realloc(total, actual_size);
            memset(total + actual_size - 1024, 0, 1024);
        }
        for (int i = 0; i < bytes; i++)
        {
            total[counter++] = buffer[i];
        }
        size_t i = strlen(total);
        // do the same for last 4
        if (total[i - 1] == '\n' && total[i - 2] == '\r')
        {
            break;
        }
    }
    respond(client_fd, total, bytes, resp);
}

void start_server(int server_socket, struct config *config)
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
            struct response *resp = create_response(config);
            communicate(cfd, resp);
            destroy_response(resp);
            close(cfd);
        }
    }
}
