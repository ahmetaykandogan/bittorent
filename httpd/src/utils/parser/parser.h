#ifndef PARSER_H
#define PARSER_H

#include "../../config/config.h"

struct response
{
    int get_or_head; //0 is get, 1 is head
    char *http_type;
    //date
    char *content; // the body
    char *file; // where the file to read is
    struct config *config;
};

struct response *create_response(struct config *config);

void parser(const char *request, struct response *fill);

void destroy_response(struct response *out);

#endif /* ! PARSER_H */
