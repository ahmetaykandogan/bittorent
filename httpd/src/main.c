#include <err.h>

#include "config/config.h"
#include "daemon/daemon.h"
#include "server/server.h"
#include "utils/parser/parser.h"
int main(int argc, char *argv[])
{
    struct config *configuration = parse_configuration(argc, argv);
    char *ip = configuration->servers->ip;
    char *port = configuration->servers->port;
    int check = daemonize(configuration);
    if (check == -1)
    {
        config_destroy(configuration);
        return 2;
    }
    // STOP signal
    if (check == 2)
    {
        config_destroy(configuration);
        return 0;
    }
    if (check == 3)
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
}
