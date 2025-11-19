#ifndef SERVER_H
#define SERVER_H

#include <unistd.h>

void start_server(int server_socket);

void respond(int client_fd, const char *buffer, ssize_t bytes);

int create_and_bind(const char *node, const char *service);

void start_server(int server_socket);

#endif /* ! SERVER_H */
