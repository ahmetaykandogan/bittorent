#include "string.h"
#include <stdlib.h>
#include <string.h>

struct string *string_create(const char *str, size_t size){
    struct string *out = calloc(1, sizeof(struct string));
    if (!out){
        return NULL;
    }
    out->size = size;
    out->data = calloc(size, sizeof(char));
    if (!out->data){
        free(out);
        return NULL;
    }
    for (size_t i = 0; i < size; i++){
        out->data[i] = str[i];
    }
    return out;
}

int string_compare_n_str(const struct string *str1, const char *str2, size_t n)
{
    for (size_t i = 0; i < n; i++){
        if (i >= str1->size){
            break;
        }
        if (i >= strlen(str2)){
            break;
        }
        if (str1->data[i] > str2[i])
        {
            return 1;
        }
        else if (str1->data[i] < str2[i])
        {
            return -1;
        }
    }
    return 0;
}

void string_concat_str(struct string *str, const char *to_concat, size_t size)
{
    str->data = realloc(str->data, str->size + size);
    for (size_t i = 0; i < size; i++){
        str->data[str->size + i] = to_concat[i];
    }
    str->size += size;
}

void string_destroy(struct string *str){
    if (str){
        free(str->data);
        free(str);
    }
}
