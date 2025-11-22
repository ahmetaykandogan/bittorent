#include "parser.h"

#include "../../config/config.h"
#include <stdlib.h>
#include <string.h>

static char *add_char(char *line, size_t index, char add)
{
    size_t len = strlen(line);
    line = realloc(line, len + 2);

    for (size_t i = len + 1; i > index; i--)
    {
        line[i] = line[i - 1];
    }
    line[index] = add;
    return line;
}
struct response *create_response(struct config *config)
{
    struct response *out = calloc(1, sizeof(struct response));
    out->http_type = calloc(1, sizeof(char));
    out->get_or_head = 0;
    if (config->servers->root_dir)
    {
        out->file = strdup(config->servers->root_dir);
    }
    out->file = calloc(1, sizeof(char));
    out->content = calloc(1, sizeof(char));
    out->config = config;
    // date;
    return out;
}
void parser(const char *request, struct response *fill)
{
    if (!request)
    {
        return;
    }
    size_t i = 0;
    while (i < strlen(request))
    {
        if (request[0] == 'H')
        {
            fill->get_or_head = 1;
        }
        while (request[i] != ' ' && i < strlen(request))
        {
            i++;
        }
        i++;
        // this means we are in the root_dir to fill part;
        if (request[i] == '/' && request[i + 1] == ' ')
        {
            //to change with config->default_file;
            fill->file = realloc(fill->file, strlen(fill->file) + strlen(fill->config->servers->default_file) + 1);
            fill->file = strcat(fill->file, fill->config->servers->default_file);
        }
        else
        {
            int place0 = 0;
            while (request[i] != ' ' && i < strlen(request))
            {
                fill->file = add_char(fill->file, place0++, request[i]);
                i++;
            }
        }
        i+=2;
        int place = 0;
        while (request[i] != '\n' && i< strlen(request) && request[i] != '\r')
        {
            fill->http_type = add_char(fill->http_type, place++, request[i]);
            i++;
        }
        return;
    }
    return;
}

void destroy_response(struct response *out)
{
    free(out->http_type);
    free(out->content);
    free(out->file);
    free(out);
}
/*
int main(int argc, char *argv[])
{
    struct config *configuration = parse_configuration(argc, argv);
    char *test = "GET / HTTP/1.1\n";
    struct response *test_struct = create_response(configuration);
    parser(test, test_struct);
    destroy_response(test_struct);
    return 0;
}
*/
