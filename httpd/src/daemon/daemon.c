#define _POSIX_C_SOURCE 200809L

#include "daemon.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
//-1 means that a PID exist and is running
// 1 means it exist and is ok to be filled
static int check_pid(FILE *file, int is_it_void)
{
    if (is_it_void != 1)
    {
        char *buf = NULL;
        size_t len = 0;
        getline(&buf, &len, file);
        int pid_to_check = atoi(buf);
        int check = kill(pid_to_check, 0);
        if (check == 0)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
    return 1;
}
static void kill_pid(FILE *file)
{
    char *buf = NULL;
    size_t len = 0;
    getline(&buf, &len, file);
    int pid_to_kill = atoi(buf);
    int check = kill(pid_to_check, SIGTERM);
    if (check == 0)
    {
        waitpid(pid_to_kill, NULL, 0);
    }
}

int daemonize(struct config *config)
{
    FILE *file = fopen(config->pid_file, "r+");
    int is_it_void = 0;
    if (!file)
    {
        file = fopen(config->pid_file, "w+");
        is_it_void = 1;
    }
    int check = check_pid(file, is_it_void);
    if (config->daemon == NO_OPTION)
    {
        return 1;
    }
    else if (config->daemon == START)
    {
        if (check == -1)
        {
            return -1;
        }
        pid_t var = fork();
        if (var)
        {
            fclose(file);
            file = fopen(config->pid_file, "w");
            fprintf(file, "%d", var);
            return 3; //so the main can understand this is the parent
        }
        return 1;
    }
    else if (config->daemon == STOP)
    {
        if (check == -1) //meaning a pid is running and need to be killed
        {
            kill_pid(file); //the pid stopped running here
            fclose(file); //close the file cause I will delete it after
        }
        remove(config->pid_file); 
        return 1;
    }
    else if (config->daemon == RESTART)
    {
        return 1;
    }
    return 1;
}
