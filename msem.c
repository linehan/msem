//#define _JDL_NO_PRINT_LOCATION
#define _JDL_NO_PRINT_DEBUG
#define _JDL_NO_PRINT_WARN

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <j/time.h>
#include <j/file.h>
#include <j/shell.h>
#include <j/debug.h>
#include "msem.h"

extern int errno;

/* 
 * After Steven's 3-member semaphore set implementation.
 *
 * Create a set of 3 semaphores:
 */

        /* [0]: The actual semaphore value. */
        #define SEMAPHORE 0
        /* [1]: The process counter.        */
        #define PROCESSES 1
        /* [2]: A lock variable for internal use. */
        #define NO_RACING 2

/*
 * [1] is initialized to a large number, then decremented on every 
 * create/open, and incremented on every close. Makes it possible to
 * use the "adjust" feature to account for processes which exit before
 * calling sem_close().
 *
 * [2] is used to avoid the race conditions caused by sem_make()
 * and sem_close(), as discussed in those functions.
 */


/* The maximum process count. */
#define BIGCOUNT 10000

/* Number of semaphores in the semaphore set. */
#define NSEMS 3



/******************************************************************************
 * OPERATIONS
 *
 * System V's semop() will perform operations on arrays of 
 * type struct sembuf, combining the sequence of operations 
 * into atomic ones.
 *
 * In many of these operations, the flag SEM_UNDO is set, which
 * causes the relevant operation to be undone in the event of
 * the process abnormally terminating.
 *
 *      struct sembuf {
 *              short sem_num   Semaphore number
 *              short sem_op    Semaphore operation
 *              short sem_flg   Operation flags
 *      }
 *
 ******************************************************************************/

/* 
 * LOCK
 * 0. Wait for NO_RACING to be 0 (unlocked).
 * 1. Increment NO_RACING to 1 (lock).
 */
#define nops_lock 2
static struct sembuf op_lock[nops_lock] = {
        {NO_RACING,   0,   0},
        {NO_RACING,   1,   SEM_UNDO}
};


/* 
 * ENDCREATE
 * 0. Decrement the PROCESSES counter.
 * 1. Unlock NO_RACING (decrement to 0). 
 */
#define nops_endcreate 2
static struct sembuf op_endcreate[nops_endcreate] = {
        {PROCESSES,   -1,  SEM_UNDO},
        {NO_RACING,   -1,  SEM_UNDO}
};


/*
 * OPEN
 * 0. Decrement PROCESSES.
 */
#define nops_open 1
static struct sembuf op_open[nops_open] = {
        {PROCESSES, -1, SEM_UNDO}
};


/* 
 * CLOSE
 * 0. Wait for NO_RACING to be 0 (unlocked).
 * 1. Increment NO_RACING to 1 (lock).
 * 2. Increment the PROCESSES counter.
 */
#define nops_close 3
static struct sembuf op_close[nops_close] = {
        {NO_RACING, 0, 0},
        {NO_RACING, 1, SEM_UNDO},
        {PROCESSES, 1, SEM_UNDO}
};


/*
 * UNLOCK
 * Decrement NO_RACING to 0 (unlock).
 */
#define nops_unlock 1
static struct sembuf op_unlock[nops_unlock] = {
        {NO_RACING, -1, SEM_UNDO}
};


/*
 * SAFE SEMAPHORE OPERATION (WITH UNDO)
 * 0. Decrement or increment SEMAPHORE by 99.
 * NOTE
 * The 99 is set to the actual amount to
 * add or subtract (positive or negative).
 */
#define nops_sem 1
static struct sembuf op_sem[nops_sem] = {
        {SEMAPHORE, 99, SEM_UNDO}
};


/*
 * UNSAFE SEMAPHORE OPERATION (NO UNDO)
 * 0. Decrement or increment SEMAPHORE by 99.
 * NOTE
 * The 99 is set to the actual amount to
 * add or subtract (positive or negative).
 */
#define nops_raw 1
static struct sembuf op_raw[nops_raw] = {
        {SEMAPHORE, 99, 0}
};


/******************************************************************************
 * SEMAPHORE CONTROL UNION 
 *
 * Filled in by semctl during certain calls, and while
 * optional, should be provided each time. For more info
 * about it, see man(3) semctl.
 * 
 ******************************************************************************/
union semun {
        int             val;     /* Semaphore value */
        struct semid_ds *buf;    /* Semaphore status struct */
        unsigned short  *array;  /* Used to set multiple semvals. */
} control;



/******************************************************************************
 * UTILITY FUNCTIONS 
 ******************************************************************************/

/**
 * msem_operation 
 * ``````````````
 * Wrap the System V semaphore function semop().
 *
 * @semid: Semaphore ID
 * @sops : Semaphore operation array
 * @nsops: Number of operations in @sops
 * Return: -1 on error, else 0.
 *
 * NOTE
 * This is intended to provide debugging assistance.
 */
int msem_operation(int semid, struct sembuf *sops, size_t nsops)
{
        if (semop(semid, sops, nsops) == 0) {
                return 0;
        }
        
        switch (errno) {
        case E2BIG: 
                ERROR("[E2BIG] The value of nsops is greater than the system-imposed maximum\n");
                break;
        case EACCES:
                ERROR("[EACCES] Permission is denied to the calling process\n");
                break;
        case EAGAIN:
                ERROR("[EAGAIN] Would suspend calling process but &IPC_NOWAIT is non-zero\n");
                break;
        case EFBIG:
                ERROR("[EFBIG] sem_num is less than 0 or >= #semaphores in this set\n");
                break;
        case EIDRM:
                ERROR("[EIDRM] Semaphore ID %d has been removed from system\n", semid);
                break;
        case EINTR:
                ERROR("[EINTR] Operation was interrupted by a signal\n");
                break;
        case EINVAL:
                ERROR("[EINVAL] Not a valid semaphore ID, or SEM_UNDO exceeds system limit\n");
                break;
        case ENOSPC:
                ERROR("[ENOSPC] Limit on processes requiring SEM_UNDO would be exceeded\n");
                break;
        case ERANGE:
                ERROR("[ERANGE] Operation would cause a semval or semadj to overflow\n");
                break;
        }

        return -1;
}


/**
 * msem_key
 * ````````
 * Create a key token from a path and tag.
 *
 * @path  : Path to the semaphore file.
 * @tag   : Project ID (must be non-zero).
 * @create: Create file at @path if none exists.
 * Return : Key suitable for semaphore use.
 */
int msem_key(char *path, int tag, bool create)
{
        key_t key;

        if (tag == 0) {
                ERROR("Project tag must be non-zero.\n");
                return -1;
        }

        if (access(path, F_OK) == -1) {
                switch (create) {
                case true:
                        if ((open(path, O_CREAT, 0666)) < 0) {
                                ERROR("(%d) Could not create file at %s\n", errno, path);
                                return -1;
                        } else {
                                DEBUG("Created new file at %s.\n", path);
                        }
                        break;
                case false:
                        DEBUG("File at %s does not exist.\n", path);
                        return -1;
                }
        }

        key = ftok(path, tag);

        if (key == IPC_PRIVATE) {
                DEBUG("Cannot create private semaphores.\n");
                return -1;
        } else if (key == (key_t)-1) {
                DEBUG("Likely ftok() error.\n");
                return -1;
        }
        
        return key;
}



/******************************************************************************
 * ACCESSOR FUNCTIONS 
 * 
 * Retreive information about an operating semaphore.
 * None of these functions require the caller to
 * lock the semaphore being investigated.
 *
 ******************************************************************************/



        
/**
 * msem_value
 * ``````````
 * Current value of the semaphore
 *
 * @semid: Semaphore ID.
 * Return: Value, -1 on error
 */
unsigned short msem_value(int semid)
{
        register int semval;

        control.val = 0;
        if ((semval = semctl(semid, SEMAPHORE, GETVAL, control)) < 0) {
                WARN("GETVAL failed.\n");
                return -1;
        }

        return semval;
}


/**
 * msem_pid
 * ````````
 * PID of the last process to modify the semaphore.
 *
 * @semid: Semaphore ID
 * Return: PID value, -1 on error
 */
pid_t msem_pid(int semid)
{
        register pid_t sempid;

        control.val = 0;
        if ((sempid = semctl(semid, SEMAPHORE, GETPID, control)) == -1) {
                WARN("GETPID failed.\n");
                return -1;
        }

        return sempid;
}


/**
 * msem_ncount
 * ```````````
 * # of processes waiting for semaphore value to be greater than current value
 *
 * @semid: Semaphore ID
 * Return: Process count, -1 on error
 */
unsigned short msem_ncount(int semid)
{
        register unsigned short semncnt;

        control.val = 0;
        if ((semncnt = semctl(semid, SEMAPHORE, GETNCNT, control)) == -1) {
                WARN("GETNCNT failed.\n");
                return -1;
        }

        return semncnt;
}


/**
 * msem_zcount
 * ```````````
 * # of processes waiting for semaphore value to be 0
 *
 * @semid: Semaphore ID.
 * Return: Process count, -1 on error 
 */
unsigned short msem_zcount(int semid)
{
        register unsigned short semzcnt;

        control.val = 0;
        if ((semzcnt = semctl(semid, SEMAPHORE, GETZCNT, control)) == -1) {
                WARN("GETZCNT failed.\n");
                return -1;
        }

        return semzcnt;
}


/**
 * msem_otime
 * ``````````
 * Time (sec. since UNIX epoch) of last semaphore operation
 *
 * @semid: Semaphore ID.
 * Return: UNIX time, -1 on error 
 */
int msem_otime(int semid)
{
        int r;

        if (control.buf == NULL) {
                control.buf = calloc(1, sizeof(struct semid_ds));
        }

        if ((semctl(semid, SEMAPHORE, IPC_STAT, control)) == -1) {
                WARN("IPC_STAT failed.\n");
                return -1;
        }

        r = (int)control.buf->sem_otime;

        if (control.buf != NULL) {
                free(control.buf);
        }

        return r;
}


/**
 * msem_ctime
 * ``````````
 * Time (sec. since UNIX epoch) of last semaphore control operation
 *
 * @semid: Semaphore ID.
 * Return: UNIX time, -1 on error 
 */
int msem_ctime(int semid)
{
        int r;

        if (control.buf == NULL) {
                control.buf = calloc(1, sizeof(struct semid_ds));
        }

        if ((semctl(semid, SEMAPHORE, IPC_STAT, control)) < 0) {
                WARN("IPC_STAT failed.\n");
                return -1;
        }

        r = (int)control.buf->sem_ctime;

        if (control.buf != NULL) {
                free(control.buf);
        }

        return r;
}



int msem_query(int semid, char *query_code)
{
        char code = query_code[0];

        switch (code) {
        case 'v':
                return (int)msem_value(semid); 
        case 'p':
                return (int)msem_pid(semid);
        case 'n':
                return (int)msem_ncount(semid);
        case 'z':
                return (int)msem_zcount(semid);
        case 'o':
                return (int)msem_otime(semid);
        case 'c':
                return (int)msem_ctime(semid);
        default:
                WARN("Invalid query_code\n");
                return -1;
        }
}


/******************************************************************************
 * FILE RECORD AND STATE HANDLING
 ******************************************************************************/

struct msem_data {
        struct {
                char tag;
                key_t key;
                int semid;
        } row[CHAR_MAX];
};



bool msem_scan(FILE *file, char *tag, key_t *key, int *semid)
{
        static long pos=-1;

        if (feof(file)) {
                pos = -1;
                return false;
        } 
        if (pos!=-1) {
                fseek(file, pos, SEEK_SET);
        }
        fscanf(file, "%c %d %d", tag, key, semid);
        pos = ftell(file);
        return true;
}

/**
 * msem_record()
 * `````````````
 * Write the semaphore ID to the file associated with it.
 */
int msem_record(char *path, char tag, key_t key, int id)
{
        FILE *file;
        if ((file = fopen(path, "a+"))) {
                fprintf(file, "%c %d %d\n", tag, key, id);
        } else {
                return -1;
        }
        fclose(file);
        return 1;
}



/******************************************************************************
 * LOW-LEVEL FUNCTIONS 
 *
 * msem_create
 * msem_remove 
 * msem_open
 * msem_close
 * msem_exists
 *
 ******************************************************************************/

/**
 * msem_create 
 * ```````````
 * Create a semaphore with a specified initial value.
 *
 * @key  : Key value (see sem_key)
 * @init : Initial value of the semaphore.
 * Return: Semaphore ID if all OK, else -1
 *
 * NOTE
 * If the semaphore already exists, it isn't initialized.
 */
int msem_create(char *path, char *tags, int init)
{
        key_t key;
        register int id;
        register char tag;
        register int semval;

        tag = tags[0];

        /*
         * Build the key used to get the semaphore ID from
         * the kernel's shared memory space. 
         */

        if ((key = msem_key(path, tag, true)) == -1) {
                ERROR("Could not fetch key for file %s[%c]\n", path, tag);
                return -1;
        }

again:

        /*
         * Try to create the semaphore. If it already exists,
         * the IP_EXCL flag will ensure that semget() returns
         * with an error.
         */

        if ((id = semget(key, NSEMS, 0777|IPC_CREAT|IPC_EXCL)) < 0) {
                //DEBUG("Semaphore exists, permission error, or tables full.\n");
                return -1;
        }

        /*
         * When the semaphore is created, we know that the value
         * of all 3 members is 0.
         *
         * There is a race condition between the semget() above and 
         * the semop() below. Another process can call sem_close() 
         * and remove the semaphore (if that process is the last one 
         * using it).
         * 
         * If the error condition occurs, we go back and create the
         * semaphore anew. 
         */
        if ((msem_operation(id, &op_lock[0], nops_lock)) == -1) {
                if (errno == EINVAL) {
                        DEBUG("Bullet caught with bare hands\n");
                        goto again;
                } else {
                        ERROR("Can't lock the semaphore.\n");
                        return -1;
                }
        }

        /*
         * Get the value of the process counter. If it equals 0, then
         * no one has initialized the semaphore yet.
         */ 
        control.val = 0;
        if ((semval = semctl(id, PROCESSES, GETVAL, control)) == -1) {
                ERROR("Failed to get process count.\n");
                return -1;
        }

        if (semval == 0) {
                /*
                 * Instead of initializing it with SETALL, which would
                 * clear the adjust value set when we locked it above,
                 * we do 2 system calls to initialize [0] and [1]. 
                 */
                control.val = init;
                if (semctl(id, SEMAPHORE, SETVAL, control) == -1) {
                        ERROR("Failed to set initial value.\n");
                        return -1;
                }

                control.val = BIGCOUNT;
                if (semctl(id, PROCESSES, SETVAL, control) == -1) {
                        ERROR("Failed to initialize process count.\n");
                        return -1;
                }
        }

        /*
         * Decrement the process counter and then release the lock.
         */
        if (msem_operation(id, &op_endcreate[0], nops_endcreate) == -1) {
                ERROR("Semaphore creation did not end cleanly.\n");
                return -1;
        }
        
        return id;
}



/**
 * msem_remove
 * ```````````
 * Remove a semaphore from shared memory.
 *
 * @id   : Semaphore ID.
 * Return: TRUE on success, else FALSE.
 */
int msem_remove(int semid)
{
        /*
         * Perform the remove operation.
         */

        control.val = 0;
        if (semctl(semid, SEMAPHORE, IPC_RMID, control) == -1) {
                WARN("Failed to remove semaphore.\n");
                return -1;
        }

        return 1;
} 


        
/**
 * msem_open
 * `````````
 * Convert a semaphore (path,uid) tuple to a semaphore id.
 *
 * @path : Filesystem path to the semaphore.
 * @tag  : Tag (character) indicating the region of the file at @path.
 * @init : Initial value to set the semaphore if it is created.
 * Return: Semaphore id value on success, -1 on error (ENOENT usually)
 */
int msem_open(char *path, char *tags, int init)
{
        register int s;
        register char tag;
        key_t key;

        tag = (char)*tags;

        /*
         * Try to create the semaphore. If the semaphore
         * already exists, sem_create will return error. 
         *
         * If the error isn't EEXIST, its an actual problem
         * and we have to quit. 
         */

        if ((s = msem_create(path, tags, init)) == -1) {
                if (errno != EEXIST) {
                        DEBUG("Create error\n");
                        return -1;
                } else {
                        DEBUG("Semaphore already exists\n");

                        /*
                         * The semaphore already exists. Build the key used 
                         * to get the semaphore ID from the kernel's shared 
                         * memory space. 
                         */

                        if ((key = msem_key(path, tag, true)) == -1) {
                                ERROR("Could not fetch key for file %s[%c]\n", path, tag);
                                return -1;
                        }

                        /*
                         * Get the semaphore ID of the existing semaphore. 
                         * If the call to sem_create failed with EEXIST,
                         * this will set the value of 's'.
                         */

                        if ((s = semget(key, NSEMS, 0)) == -1) {
                                ERROR("Still could not open semaphore\n");
                                return -1;
                        }
                }
        } else {
                DEBUG("Created new semaphore %s[%c] with value %d\n", path, tag, init);
                msem_record(path, tag, key, s);
        }

        /*
         * Perform the open operation to provide the semaphore
         * to your process. 
         */

        if (msem_operation(s, &op_open[0], nops_open) == -1) {
                ERROR("Semaphore operation failed\n");
                return -1;
        }

        return s;
}


/**
 * msem_close
 * ``````````
 * Close a semaphore (used by exiting process).
 *
 * @semid: Semaphore ID.
 * Return: TRUE on success, else FALSE.
 */
int msem_close(int semid)
{
        register int semval;

        /* 
         * Get a lock on the semaphore, then increment [1],
         * the process counter.
         */

        if (msem_operation(semid, &op_close[0], nops_close) == -1) {
                ERROR("Failed to close semaphore.\n");
                return -1;
        }
        
        /* 
         * Read the value of the process counter to see if this
         * is the last reference to the semaphore.
         *
         * Causes the race condition discussed in sem_make().
         */

        control.val = 0;
        if ((semval = semctl(semid, PROCESSES, GETVAL, control)) == -1) {
                ERROR("Failed to get process count.\n");
                return -1;
        }

        if (semval > BIGCOUNT) {
                ERROR("Process count (somehow) exceeds minimum.\n");
                return -1;
        } else if (semval == BIGCOUNT) {
                WARN("Last process using semaphore. Removing semaphore.\n");
                msem_remove(semid);
                return 0;
        } else {
                if (msem_operation(semid, &op_unlock[0], nops_unlock) == -1) {
                        ERROR("Failed to unlock semaphore.\n");
                        return -1;
                }
        }

        /* Decremented successfully */
        return semval;
}

/**
 * msem_exists()
 * `````````````
 * @path : Filesystem path to the semaphore.
 * @tag  : Tag (character) indicating the region of the file at @path.
 * Return: TRUE if semaphore exists at @path:@tag, else FALSE
 */
int msem_exists(char *path, char *tags)
{
        key_t key;
        int id;
        char tag;

        tag = (char)*tags;

        if ((key = ftok(path, tag)) == -1) {
                return false;
        }
        if ((id = semget(key, 0, 0777)) == -1) {
                return false;
        }
        if (shell("ipcs -s | awk '{print $2}' | grep -q %d", id)) {
                return false;
        }
        return true;
}
 


/******************************************************************************
 * TIMEOUT SUPPORT 
 ******************************************************************************/

/* 
 * Set by the SIGALRM handler. Allows the caller 
 * to distinguish a timeout from some other error 
 * in the semaphore. 
 */
volatile bool CAUGHT_ALARM = false;


/**
 * sem_alarm_handler
 * `````````````````
 * Handler for SIGALRM signal.
 *
 * @signo: Signal number (should be SIGALRM).
 * Return: Nothing.
 */
void msem_alarm_handler(int sig)
{
        CAUGHT_ALARM = 1;
}


/**
 * sem_alarm_ms 
 * ```````````` 
 * Set a delayed SIGALRM signal.
 *
 * @ms   : Number of milliseconds to delay.
 * Return: 1 on success, -1 on error.
 */
int set_alarm(int ms)
{
        struct itimerval when = {{0},{0}};

        signal(SIGALRM, &msem_alarm_handler);

        when.it_interval.tv_sec  = 0;
        when.it_interval.tv_usec = 0;
        when.it_value.tv_sec     = ms / SEC_IN_MS;
        when.it_value.tv_usec    = MS_TO_US((ms%SEC_IN_MS));

        /*DEBUG("tv_sec:  %ld\ntv_nsec: %ld\n", when.tv_sec, when.tv_nsec);*/

        if ((setitimer(ITIMER_REAL, &when, 0)) == EINVAL) {
                ERROR("Timer value not canonicalized.\n");
                return -1;
        }

        DEBUG("Programmed thread death in %dms.\n", ms);

        return 1;
}


/******************************************************************************
 * SEMAPHORE OPERATIONS 
 ******************************************************************************/

/**
 * msem_set_once
 * `````````````
 * Alter a semaphore's value by some amount.
 *
 * @semid: Semaphore ID
 * @value: Value to move semaphore.
 * @ms   : Milliseconds before timeout.
 * Return: -1 on error, return value of semop on success.
 *
 * NOTE
 * If the value of @ms is <= 0, no timeout will be
 * set. 
 */
int msem_set_once(int semid, int value, int ms)
{
        pid_t pid = 1;

        /*
         * No operation is defined for value 0, so
         * we have to prevent this being argued.
         */

        if ((op_raw[0].sem_op = value) == 0) {
                WARN("Semaphore operation value 0 not permitted.\n");
                return -1;
        }

        /*
         * If there are a valid number of milliseconds
         * to wait, set the alarm handler before beginning
         * the operation.
         */

        if (ms > 0) {
                if ((set_alarm(ms)) == -1) {
                        ERROR("Could not establish timer, aborting.\n");
                        return -1;
                }
        }

        /*
         * If the operation returns with an error, and the 
         * CAUGHT_ALARM global variable was set, the operation 
         * timed out.
         */

        if ((msem_operation(semid, &op_raw[0], nops_raw)) == -1) {
                if (CAUGHT_ALARM == true) {
                        DEBUG("Caught SIGALRM (timed out).\n");
                        pid = (int)getpid();
                        DEBUG("[%d] Programmed thread death.\n", pid);
                        CAUGHT_ALARM = false;
                        return -1;
                } else {
                        WARN("Semaphore operation failed.\n");
                        return -1;
                }
        }

        return pid;
}


/**
 * msem_set_safe
 * `````````````
 * Alter a semaphore's value by some amount.
 *
 * @semid: Semaphore ID
 * @value: Value to move semaphore.
 * Return: -1 on error, return value of semop on success.
 *
 * NOTE
 * If the value of @ms is <= 0, no timeout will be
 * set. 
 *
 * If the process dies while holding a semaphore, it will
 * undo any semaphore tokens it decremented. 
 */
int msem_set_safe(int semid, int value, int ms)
{
        pid_t pid = 1;

        /*
         * No operation is defined for value 0, so
         * we have to prevent this being argued.
         */

        if ((op_sem[0].sem_op = value) == 0) {
                WARN("Semaphore operation value 0 not permitted.\n");
                return -1;
        }

        /*
         * If there are a valid number of milliseconds
         * to wait, set the alarm handler before beginning
         * the operation.
         */

        if (ms > 0) {
                if ((set_alarm(ms)) == -1) {
                        ERROR("Could not establish timer, aborting.\n");
                        return -1;
                }
        }

        /*
         * If the operation returns with an error, and the 
         * CAUGHT_ALARM global variable was set, the operation 
         * timed out.
         */

        if ((msem_operation(semid, &op_sem[0], nops_sem)) == -1) {
                if (CAUGHT_ALARM == true) {
                        msem_close(semid);
                        DEBUG("Caught SIGALRM (timed out).\n");
                        pid = (int)getpid();
                        DEBUG("[%d] Programmed thread death.\n", pid);
                        CAUGHT_ALARM = false;
                        return -1;
                } else {
                        WARN("Semaphore operation failed.\n");
                        return -1;
                }
        }

        return pid;
}







/******************************************************************************
 * HANDY ONE-FUNCTION INTERFACE 
 ******************************************************************************/

int msem(int semid, char *mode, int timeout)
{
        register int n = -1;
        register int r = -1;

        switch (mode[0]) {
        case '-':
        case 'p':
                WARN("[%d] '- or p' (lock)\n", semid);
                switch (mode[1]) {
                case ',':
                        WARN("[%d] '-,' (lock with undo)\n", semid);
                        r = msem_set_safe(semid, -1, timeout);
                        break;
                case '\0':
                        WARN("[%d] '-' (lock)\n", semid);
                        r = msem_set_once(semid, -1, timeout);
                        break;
                }
                break;
        case '+':
        case 'v':
                WARN("[%d] '+ or v' (unlock)\n", semid);
                switch (mode[1]) {
                case '*':
                        n = msem_query(semid, "n");
                        WARN("[%d] '+*' (relax %d)\n", semid, n);
                        r = msem_set_once(semid, n, 0);
                        break;
                case ',':
                        WARN("[%d] '+,' (unlock with undo)\n", semid);
                        r = msem_set_safe(semid, 1, 0);
                        break;

                case '\0':
                        WARN("[%d] '+' (unlock)\n", semid);
                        r = msem_set_once(semid, 1, 0);
                        break;
                }
                break;
        default:
                WARN("Invalid mode supplied\n");
                return -1;
        }

        return (r == 0 || r == -1) ? (int)false : (int)true;
}


