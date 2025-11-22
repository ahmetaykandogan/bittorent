#ifndef SERVER_H
#define SERVER_H

#include <unistd.h>
#include "../utils/parser/parser.h"

int create_and_bind(const char *node, const char *service);

void start_server(int server_socket, struct config *config);

#endif /* ! SERVER_H */
