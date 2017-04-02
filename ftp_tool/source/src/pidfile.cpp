#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "pidfile.h"

/* Create the running daemon pidfile */
bool pidfile_write(const char *pid_file, int pid)
{
    FILE *fp = fopen(pid_file, "w");

    if (NULL == fp)
    {
        std::cout << "pidfile_write: Cannot open " << pid_file << " pidfile" << std::endl;
        return false;
    }

    fprintf(fp, "%d\n", pid);
    fclose(fp);
    return true;
}

/* Remove the running daemon pidfile */
void pidfile_rm(const char *pid_file)
{
    unlink(pid_file);
}

/* Return the daemon running state */
pid_t process_running(const char *pid_file)
{
    FILE *fp = fopen(pid_file, "r");
    pid_t pid;

    /* No pidfile */
    if (NULL == fp)
        return 0;

    fscanf(fp, "%10d", &pid);
    fclose(fp);

    /* If no process is attached to pidfile, remove it */
    if (kill(pid, 0) != 0)
    {
        std::cout << "Remove a zombie pid file " << pid_file << std::endl;
        pidfile_rm(pid_file);
        return 0;
    }

    return pid;
}
