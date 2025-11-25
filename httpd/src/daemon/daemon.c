#define _POSIX_C_SOURCE 200809L

#include "daemon.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static int read_line(FILE *file)
{
    rewind(file);

    char *buf = NULL;
    size_t len = 0;

    if (getline(&buf, &len, file) == -1)
    {
        free(buf);
        return -1;
    }

    int pid_to_check = atoi(buf);
    free(buf);
    return pid_to_check;
}

static int check_pid(int pid, int is_it_void)
{
    if (is_it_void != 1)
    {
        int check = kill(pid, 0);
        if (check == 0)
            return -1;
        else
            return 1;
    }
    return 1;
}

static void kill_pid(int pid)
{
    kill(pid, SIGTERM);
}

int daemonize(struct config *config)
{
    if (config->daemon == NO_OPTION)
        return 1;

    FILE *file = fopen(config->pid_file, "r+");
    int is_it_void = 0;
    int pid = -1;
    if (!file)
    {
        file = fopen(config->pid_file, "w+");
        is_it_void = 1;
    }
    else
    {
        pid = read_line(file);
    }
    int check = check_pid(pid, is_it_void);
    if (config->daemon == START)
    {
        if (check == -1)
            return -1;
        pid_t var = fork();
        if (var)
        {
            fclose(file);
            file = fopen(config->pid_file, "w");
            fprintf(file, "%d\n", var);
            fclose(file);
            return 3;
        }
        return 1;
    }
    else if (config->daemon == STOP)
    {
        if (check == -1)
            kill_pid(pid);

        fclose(file);
        remove(config->pid_file);
        return 2;
    }
    else if (config->daemon == RESTART)
    {
        if (check == -1)
            kill_pid(pid);

        fclose(file);
        remove(config->pid_file);

        pid_t var = fork();
        if (var)
        {
            file = fopen(config->pid_file, "w");
            fprintf(file, "%d\n", var);
            fclose(file);
            return 3;
        }
        return 1;
    }

    return 1;
}
