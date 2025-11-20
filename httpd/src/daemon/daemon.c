#define _POSIX_C_SOURCE 200809L

#include "daemon.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
//-1 means that a PID exist and is running
// 1 means it exist and is ok to be filled
static int read_line(FILE *file, char *buf)
{
    size_t len = 0;
    getline(&buf, &len, file);
    int pid_to_check = atoi(buf);
    free(buf);
    return pid_to_check;
}
static int check_pid(int pid, int is_it_void)
{
    if (is_it_void != 1)
    {
        char *buf = NULL;
        int check = kill(pid, 0);
        free(buf);
        if (check == 0)
        {
            return -1;
        }
        else
        {
            return 1;
        }
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
    {
        return 1;
    }
    FILE *file = fopen(config->pid_file, "r+");
    int is_it_void = 0;
    char *buf = NULL;
    int pid;
    if (!file)
    {
        file = fopen(config->pid_file, "w+");
        is_it_void = 1;
    }
    else{
        pid = read_line(file, buf);
    }
    int check = check_pid(pid, is_it_void);
    if (config->daemon == START)
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
            fclose(file);
            return 3; //so the main can understand this is the parent
        }
        return 1;
    }
    else if (config->daemon == STOP)
    {
        if (check == -1) //meaning a pid is running and need to be killed
        {
            kill_pid(pid); //the pid stopped running here
        }
        fclose(file); //close the file cause I will delete it after
        remove(config->pid_file); 
        return 2;
    }
    else if (config->daemon == RESTART)
    {
        if (check == -1) //meaning a pid is running and need to be killed
        {
            kill_pid(pid); //the pid stopped running here
        }
        fclose(file); //close the file cause I will delete it after
        remove(config->pid_file); 
        pid_t var = fork();
        if (var)
        {
            file = fopen(config->pid_file, "w");
            fprintf(file, "%d", var);
            return 3; //so the main can understand this is the parent
        }
        return 1;
    }
    return 1;
}
