#include "config.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/string/string.h"

static int fill_struct(struct config *out, struct server_config *server,
                       int token, char *optarg)
{
    if (token == 'p')
    {
        out->pid_file = strdup(optarg);
    }
    else if (token == 'f')
    {
        out->log_file = strdup(optarg);
    }
    else if (token == 'l')
    {
        if (strcmp(optarg, "true"))
        {
            out->log = false;
        }
        else
        {
            out->log = true;
        }
    }
    else if (token == 's')
    {
        server->server_name = string_create(optarg, strlen(optarg));
    }
    else if (token == 'o')
    {
        server->port = strdup(optarg);
    }
    else if (token == 'i')
    {
        server->ip = strdup(optarg);
    }
    else if (token == 'r')
    {
        server->root_dir = strdup(optarg);
    }
    else if (token == 'd')
    {
        server->default_file = strdup(optarg);
    }
    else if (token == 'x')
    {
        if (strcmp(optarg, "stop") == 0)
        {
            out->daemon = STOP;
        }
        else if (strcmp(optarg, "start") == 0)
        {
            out->daemon = START;
        }
        else if (strcmp(optarg, "restart") == 0)
        {
            out->daemon = RESTART;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

static int check(struct config *out)
{
    struct server_config *server = out->servers;

    if (!server->default_file)
    {
        server->default_file = strdup("index.html");
    }

    if (!out->pid_file || !server->server_name || !server->port || !server->ip
        || !server->root_dir)
    {
        return -1;
    }

    return 0;
}

struct config *parse_configuration(int argc, char *argv[])
{
    optind = 1;
    struct config *out = calloc(1, sizeof(struct config));
    if (!out)
    {
        return NULL;
    }
    out->log = true;
    out->daemon = NO_OPTION;

    out->servers = calloc(1, sizeof(struct server_config));
    if (!out->servers)
    {
        free(out);
        return NULL;
    }
    struct server_config *server = out->servers;
    static struct option long_options[] = {
        { "pid_file", required_argument, NULL, 'p' },
        { "log_file", required_argument, NULL, 'f' },
        { "log", required_argument, NULL, 'l' },
        { "server_name", required_argument, NULL, 's' },
        { "port", required_argument, NULL, 'o' },
        { "ip", required_argument, NULL, 'i' },
        { "root_dir", required_argument, NULL, 'r' },
        { "default_file", required_argument, NULL, 'd' },
        { "daemon", required_argument, NULL, 'x' },
        { 0, 0, 0, 0 }
    };
    int token;
    while ((token = getopt_long(argc, argv, "", long_options, NULL)) != -1)
    {
        if (fill_struct(out, server, token, optarg) != 0)
        {
            config_destroy(out);
            return NULL;
        }
    }
    if (check(out) != 0)
    {
        config_destroy(out);
        return NULL;
    }
    return out;
}

void config_destroy(struct config *config)
{
    if (!config)
    {
        return;
    }
    free(config->pid_file);
    free(config->log_file);
    struct server_config *server = config->servers;
    if (server)
    {
        if (server->server_name)
        {
            string_destroy(server->server_name);
        }
        if (server->port)
            free(server->port);
        if (server->ip)
            free(server->ip);
        if (server->root_dir)
            free(server->root_dir);
        if (server->default_file)
            free(server->default_file);
        free(server);
    }
    free(config);
}
