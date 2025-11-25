#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "../../config/config.h"

void parser(const char *req, struct response *fill)
{
    if (!req || !fill)
        return;

    size_t i = 0;
    if (strncmp(req, "GET ", 4) == 0)
        fill->get_or_head = 0;
    else if (strncmp(req, "HEAD ", 5) == 0)
        fill->get_or_head = 1;
    else
    {
        fill->get_or_head = -1;
        return;
    }

    while (req[i] && req[i] != ' ')
        i++;
    if (req[i] != ' ')
        return;
    i++;
    if (req[i] != '/')
        return;
    i++;
    if (req[i] == ' ')
    {
        const char *df = fill->config->servers->default_file;
        if (!df)
            df = "index.html";
        fill->file = realloc(fill->file, strlen(fill->file) + strlen(df) + 2);
        strcat(fill->file, "/");
        strcat(fill->file, df);
    }
    else
    {
        size_t len = strlen(fill->file);
        fill->file = realloc(fill->file, len + 2);
        fill->file[len] = '/';
        fill->file[len + 1] = '\0';
        while (req[i] && req[i] != ' ')
        {
            size_t cur = strlen(fill->file);
            fill->file = realloc(fill->file, cur + 2);
            fill->file[cur] = req[i];
            fill->file[cur + 1] = '\0';
            i++;
        }
    }

    if (req[i] != ' ')
        return;
    i++;
    if (strncmp(req + i, "HTTP/1.1", 8) == 0)
    {
    }
    else
    {
        free(fill->http_type);
        fill->http_type = strdup("HTTP/1.1");
        fill->get_or_head = -2;
        return;
    }
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
