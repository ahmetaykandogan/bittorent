#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "config/config.h"
#include "daemon/daemon.h"
#include "server/server.h"
#include "utils/parser/parser.h"

int main(int argc, char *argv[])
{
    struct sigaction siga;
    siga.sa_flags = 0;
    siga.sa_handler = sk;
    if (sigemptyset(&siga.sa_mask) < 0)
    {
        return 2;
    }
    sigaction(SIGINT, &siga, NULL);
    sigaction(SIGPIPE, &siga, NULL);
    struct config *configuration = parse_configuration(argc, argv);
    if (!configuration)
    {
        return 2;
    }
    char *ip = configuration->servers->ip;
    char *port = configuration->servers->port;
    int check = daemonize(configuration);
    if (check == -1)
    {
        config_destroy(configuration);
        return 1;
    }
    if (check == 2 || check == 3)
    {
        config_destroy(configuration);
        return 0;
    }
    int socket_fd = create_and_bind(ip, port);
    if (socket_fd == -1)
    {
        config_destroy(configuration);
        errx(1, "somethong happened");
    }
    start_server(socket_fd, configuration);
    close(socket_fd);
    config_destroy(configuration);
    return 0;
}
