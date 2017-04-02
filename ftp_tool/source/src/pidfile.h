#ifndef _PIDFILE_H_
#define _PIDFILE_H_

/* Create the running daemon pidfile */
bool pidfile_write(const char *pid_file, int pid);

/* Remove the running daemon pidfile */
void pidfile_rm(const char *pid_file);

/* Return the daemon running state */
pid_t process_running(const char *pid_file);

#endif
