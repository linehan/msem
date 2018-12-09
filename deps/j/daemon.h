#ifndef _DAEMON_LIBRARY_H
#define _DAEMON_LIBRARY_H
#include <unistd.h>

struct ids_t {
        pid_t process;
        pid_t pgroup;
        pid_t parent;
        gid_t group;
        uid_t user;
};

extern void get_ids(struct ids_t *id);
extern void set_ids(struct ids_t *id);


/* PID management */
extern int pidfile(const char *path, const char *mode);
extern void pidw(const char *path, int pid);
extern int  pidr(const char *path);


/* Process grooming */ 
extern int fork_daemon(void);
extern int fork_coprocess(void);


#endif
