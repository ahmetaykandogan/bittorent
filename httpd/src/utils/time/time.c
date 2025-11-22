#include "time.h"

#include <stdio.h>
#include <time.h>

char *print_date(char *out)
{
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = gmtime(&rawtime);
    strftime(out, 80, "Date: %a, %d %b %Y %H:%M:%S %Z\n", info);
    // printf("%s\n", timeStr);
    return out;
}
/*
int main(void){
    char out[80];
    char *out2 = print_date(out);
    printf("%s", out2);
    return 0;
}
*/
