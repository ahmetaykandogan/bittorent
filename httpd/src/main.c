#include "config/config.h"
#include "server/server.h"
#include "daemon/daemon.h"

#include <err.h>
int main(int argc, char *argv[])
{
    struct config *configuration = parse_configuration(argc, argv);
    char *ip = configuration->servers->ip;
    char *port = configuration->servers->port;
    int check = daemonize(configuration);
    if (check == -1){
        return 2;
    }
    if (check == 3){
        return 0;
    }
    int socket_fd = create_and_bind(ip, port);
    if (socket_fd == -1)
    {
        errx(1, "somethong happened");
    }
    start_server(socket_fd);
    close(socket_fd);
}
