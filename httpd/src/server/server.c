#define _POSIX_C_SOURCE 200809L
#include "server.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../utils/parser/parser.h"
#include "../utils/time/time.h"

#define BUFFER_SIZE 1024

static int signal_kill = 0;

void sk(int sig)
{
    // printf("Entered sk\n");
    if (sig == SIGINT)
    {
        signal_kill = 1;
    }
    else if (sig == SIGPIPE)
    {
        return;
    }
    return;
}

struct response *create_response(struct config *config)
{
    struct response *out = calloc(1, sizeof(*out));
    if (!out)
    {
        return NULL;
    }
    out->http_type = strdup("HTTP/1.1");
    out->get_or_head = 0;
    out->content = NULL;
    if (config && config->servers && config->servers->root_dir)
    {
        out->file = strdup(config->servers->root_dir);
    }
    else
    {
        out->file = strdup("");
    }
    out->config = config;
    return out;
}

static int build_output(char *out, size_t max, const char *httptype,
                        const char *status, size_t content_length)
{
    char date[200];
    print_date(date);
    return snprintf(out, max,
                    "%s %s\r\n"
                    "%s\r\n"
                    "Content-Length: %zu\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    httptype, status, date, content_length);
}

static void send_total(int file, const char *buf, size_t len)
{
    size_t total = 0;
    while (total < len)
    {
        ssize_t s = send(file, buf + total, len - total, 0);
        if (s <= 0)
        {
            return;
        }
        total += s;
    }
}

static void error(int file, struct response *resp, const char *status,
                  const char *in)
{
    char out[1000];
    size_t len = strlen(in);
    int out_size = build_output(out, sizeof(out), resp->http_type, status, len);
    if (out_size > 0)
    {
        send_total(file, out, out_size);
    }
    if (resp->get_or_head == 1)
    {
        return;
    }
    send_total(file, in, len);
}

void respond(int client_fd, struct response *resp)
{
    if (!resp)
    {
        return;
    }
    if (resp->get_or_head == -1)
    {
        error(client_fd, resp, "405 Method Not Allowed", "");
        return;
    }
    if (resp->get_or_head == -2)
    {
        error(client_fd, resp, "505 HTTP Version Not Supported",
              "HTTP Version Not Supported\n");
        return;
    }
    int file = open(resp->file, O_RDONLY);
    if (file == -1)
    {
        if (errno == EACCES)
        {
            error(client_fd, resp, "403 Forbidden", "");
        }
        else
        {
            error(client_fd, resp, "404 Not Found", "");
        }
        return;
    }
    struct stat stats;
    stat(resp->file, &stats);
    char out[1000];
    int out_size = build_output(out, sizeof(out), resp->http_type, "200 OK",
                                stats.st_size);
    if (out_size > 0)
        send_total(client_fd, out, out_size);

    if (resp->get_or_head == 1)
    {
        close(file);
        return;
    }
    off_t offset = 0;
    ssize_t remain = stats.st_size;
    while (remain > 0)
    {
        ssize_t s = sendfile(client_fd, file, &offset, remain);
        if (s <= 0)
        {
            break;
        }
        remain -= s;
    }

    close(file);
}

int create_and_bind(const char *node, const char *service)
{
    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = NULL;
    if (getaddrinfo(node, service, &hints, &res) != 0)
        return -1;

    int sockfd = -1;
    for (struct addrinfo *p = res; p != NULL; p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1)
            continue;
        int yes = 1;
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
    char buffer[BUFFER_SIZE];
    char *total = calloc(1024, 1);
    if (!total)
        return;

    int actual_size = 1024;
    int total_bytes = 0;
    int counter = 0;

    while ((bytes = recv(client_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        total_bytes += bytes;

        if (total_bytes > actual_size)
        {
            actual_size += 1024;
            char *tmp = realloc(total, actual_size);
            if (!tmp)
            {
                free(total);
                return;
            }
            total = tmp;
            memset(total + actual_size - 1024, 0, 1024);
        }

        int i;
        for (i = 0; i < bytes; i++)
            total[counter++] = buffer[i];

        total[counter] = '\0';

        if (strstr(total, "\r\n\r\n"))
            break;
    }

    parser(total, resp);
    respond(client_fd, resp);
    free(total);
}

void start_server(int server_socket, struct config *config)
{
    if (listen(server_socket, SOMAXCONN) == -1)
        return;
    while (signal_kill == 0)
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
