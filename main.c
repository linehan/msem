#define _JDL_NO_PRINT_LOCATION
#define _JDL_NO_PRINT_DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <j/file.h>
#include <j/textutils.h>
#include <j/time.h>
#include <j/shell.h>
#include <j/debug.h>
#include <j/daemon.h>
#include <j/bnfop.h>
#include <j/tty.h>
#include "msem.h"

/*
 * The string may begin with an arbitrary amount of white space 
 * (as determined by isspace(3)) followed by a single optional 
 * '+' or '-' sign.  
 *
 * If base is zero or 16, the  string  may  then include a "0x" 
 * prefix, and the number will be read in base 16; otherwise, a 
 * zero base is taken as 10 (decimal) unless the next character 
 * is '0', in which case it is taken as 8 (octal).
 *
 * The remainder of the string is converted to a long int value 
 * in the obvious manner, stopping at the first character which 
 * is not a valid digit in the given base. In bases above 10, 
 * the letter 'A' in either upper or lower case represents 10, 
 * 'B' represents 11, and so forth, with 'Z' representing 35.
 * 
 * If endptr is not NULL, strtol() stores the address of the first 
 * invalid character in *endptr.  If there were no digits at all, 
 * strtol() stores the original value of nptr in *endptr (and returns 
 * 0).  
 *
 * In particular, if *nptr is not '\0' but **endptr is '\0' on 
 * return, the entire string is valid.
 *
 * The strtoll() function works just like the strtol() function but 
 * returns a long long integer value.
 *
 * RETURN VALUE
 * The strtol() function returns the result of the conversion, 
 * unless the value would underflow or overflow. If an underflow 
 * occurs, strtol() returns LONG_MIN. If an overflow occurs, strtol() 
 * returns LONG_MAX. 
 * 
 * In both cases, errno is set to ERANGE. Precisely the same holds 
 * for strtoll() (with LLONG_MIN and LLONG_MAX instead of LONG_MIN 
 * and LONG_MAX).
 */
bool is_integer(char *str)
{
        long int result;
        char *end;

        result = strtol(str, &end, 10);

        if (errno == ERANGE) {
                switch (result) {
                case LONG_MAX:
                        DEBUG("Integer overflow\n"); 
                        return false;
                case LONG_MIN:
                        DEBUG("Integer underflow\n"); 
                        return false;
                }
        }

        if (*str!='\0' && *end=='\0') {
                return true;
        }

        return false;
}

int msem_ls_file(char *path, char *with_tag)
{
        FILE *file;
        char tag;
        int semid;
        key_t key;

        if (!(file = fopen(path, "w+"))) {
                return -1;
        }
        
        while (msem_scan(file, &tag, &key, &semid)) {
                fprintf(stderr, "Hohoho\n");
                if (with_tag==NULL || tag==*with_tag) {
                        printf("[%c] key:%x semid:%d\n", tag, (int)key, semid);
                }
        }

        fclose(file);

        return 1;
}

int msem_rm_file(char *path, char *with_tag)
{ 
        FILE *file;
        char tag;
        int semid;
        key_t key;

        if (!(file = fopen(path, "w+"))) {
                return -1;
        }
        
        while (msem_scan(file, &tag, &key, &semid)) {
                if (with_tag==NULL || tag==*with_tag) {
                        DEBUG("%c:%d removed\n", tag, semid);
                        shell("ipcrm -s %d", semid);
                }
        }
        
        if (with_tag==NULL) {
                ftruncate(fileno(file), 0);
        }

        fclose(file);

        return 1;
}



/**
 * msem_status
 * ```````````
 * Print the status of the given semaphore.
 *
 * @path : Path to the semaphore file.
 * @tag  : Tag (character) indicating the region of the file at @path.
 * @loop : Should the output be followed (like tail -f).
 * Return: 1 on success, -1 on error.
 */
int msem_status(char *path, char *user_tag, bool loop)
{
        const char *default_tag = "abcdefghijklmnopqrstuvwxyz";
        char *tag; 
        int i;
        int sem;
        int len;
        int count;
        int key;
        int line=0;
        enum cmd { NONE, RELAX, INC, DEC } command = NONE;

        if (user_tag == NULL) {
                tag = default_tag;
        } else {
                tag = user_tag;
        }

        len = strlen(tag);

        nc_start();
        nc_set(ECHO_INPUT, false);
        nc_set(SHOW_CURSOR, false);
        nc_set(BLOCKING_INPUT, false);

        mvprintw(1, 1, "Semaphore at %s:%s", path, tag);
        mvprintw(3, 1, "key   val   wait incr.   wait zero");
        refresh();

        do {
                for (i=0, count=0; i<len; i++) {
                        if (msem_exists(path, &tag[i])) {
                                if ((sem = msem_open(path, &tag[i], 0)) > 0) {
                                        move(5+count, 1);
                                        clrtoeol();

                                        if (count == line) {
                                                switch (command) {
                                                case NONE:                      break;
                                                case RELAX: msem(sem, "+*", 0); break;
                                                case INC:   msem(sem, "+",  0); break;
                                                case DEC:   msem(sem, "-",  0); break;
                                                }

                                                command = NONE;
                                                attron(A_REVERSE);
                                        }

                                        mvprintw(5+count, 1, "%-3c   %-3d   %-10d   %-4d", tag[i], msem_query(sem, "v"), msem_query(sem, "n"), msem_query(sem, "z"));

                                        if (count == line) {
                                                attroff(A_REVERSE);
                                        }

                                        count++;
                                        refresh();
                                }
                        }
                }
                refresh();

                if ((key = getch()) != ERR) {
                        switch (key) {
                        case 'j':
                        case 'J':
                        case KEY_DOWN:
                                line = (line<(count-1)) ? (line+1) : (count-1);
                                break;
                        case 'k':
                        case 'K':
                        case KEY_UP:
                                line = (line>0) ? (line-1) : 0;
                                break;
                        case ' ':
                                command = RELAX; 
                                break;
                        case 'h':
                        case 'H':
                                command = DEC;
                                break;
                        case 'l':
                        case 'L':
                                command = INC;
                                break;
                        }
                }

        } while (loop);

        return 1;
}


/******************************************************************************
 * COMMAND LINE INTERFACE 
 ******************************************************************************/

int main(int argc, char *argv[])
{
        char *path = NULL;
        char *tag = NULL;
        char *ini = NULL;
        char *timeout = NULL;
        char *semid = NULL;
        int s=-1;
        int r;



        if (bnf("msem -c <path> <tag> <ini>", &path, &tag, &ini)) {
                return msem_create(path, tag, atoi(ini));
        }
        if (bnf("msem -d <path> <tag>", &path, &tag)) {
                s = msem_open(path, tag, 0);
                r = msem_remove(s);
                goto done;
        }
        if (bnf("msem -p <path> <tag> <timeout>", &path, &tag, &timeout)) {
                s = msem_open(path, tag, 0);
                r = msem(s, "-", atoi(timeout));
                goto done;
        }
        if (bnf("msem -p, <path> <tag> <timeout>", &path, &tag, &timeout)) {
                s = msem_open(path, tag, 0);
                r = msem(s, "-,", atoi(timeout));
                goto done;
        }
        if (bnf("msem -v <path> <tag>", &path, &tag)) {
                s = msem_open(path, tag, 0);
                r = msem(s, "+", 0);
                goto done;
        }
        if (bnf("msem -v, <path> <tag>", &path, &tag)) {
                s = msem_open(path, tag, 0);
                r = msem(s, "+,", 0);
                goto done;
        }
        if (bnf("msem -v+ <path> <tag>", &path, &tag)) {
                s = msem_open(path, tag, 0);
                r = msem(s, "+*", 0);
                goto done;
        }

        /* Follow semaphore status. */
        if (bnf("msem -f <path> [<tag>]", &path, &tag)) {
                msem_status(path, tag, true);
                return 1;
        }

        /* List active semaphores */
        if (bnf("msem -l [<path>]", &path)) {
                if (path) {
                        return msem_ls_file(path, NULL);
                } else {
                        return mecho("ipcs -s");
                }
        }

        /* Remove an active semaphore */
        if (bnf("msem rm <semid>", &semid)) {
                if (!strcmp(semid, "all")) {
                        return mecho(
                                "for id in `ipcs -s | tail -n +4 | grep ' 777 ' | awk '{print $2}'`; "
                                "do "
                                        "ipcrm -s $id; "
                                        "echo \"($id) removed\"; "
                                "done"
                        );
                } else if (is_integer(semid)) {
                        return mecho("ipcrm -s %d", atoi(semid));
                } else if (access(semid, F_OK) != -1) {
                        return msem_rm_file(semid, NULL);
                } else {
                        return -1;
                }
        }

        /* Like msem -l */
        if (bnf("msem $")) {
                return mecho("ipcs -s");
        }

done:
        fprintf(stderr, "farewell!\n");
        msem_close(s);
        return 1;
}

