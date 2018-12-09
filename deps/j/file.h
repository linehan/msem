#ifndef _MY_FILE_LIB_H
#define _MY_FILE_LIB_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>


/* Limits 
``````````````````````````````````````````````````````````````````````````````*/
#define PATHSIZE 256
#define LINESIZE 1024

#define PERMS (0666)
#define DIR_PERMS ((S_IRWXU | S_IRGRP | S_IROTH | S_IXOTH))

#define BYTES_IN_KILOBYTE 1024
#define BYTES_IN_MEGABYTE 1048576

#define size_kb(bytes) bytes * BYTES_IN_KILOBYTE
#define size_mb(bytes) bytes * BYTES_IN_MEGABYTE


DIR *dpath(char *op, const char *fmt, ...);

extern void unlink_path(const char *path);

/* Raw file descriptors 
``````````````````````````````````````````````````````````````````````````````*/
extern int  fd_create(const char *path, int oflag, int perms);
extern int  fd_open  (const char *path, int oflag, ...);
extern void fd_close (int fd);


/* Buffered IO streams
``````````````````````````````````````````````````````````````````````````````*/
extern FILE *fs_open (const char *path, const char *mode);
extern void  fs_close(FILE *file);


/* Directory structures
``````````````````````````````````````````````````````````````````````````````*/
extern void dir_create(const char *path, int perms);
extern void dir_remove(const char *path);
extern DIR *dir_open  (const char *path);
extern void dir_close (DIR *dir);


/* Named pipes (FIFOs)
``````````````````````````````````````````````````````````````````````````````*/
extern void fifo_create(const char *path, int perm);
extern void fifo_remove(const char *path);
extern int  fifo_open  (const char *path, int oflag, ...);
extern void fifo_close (int fd);

extern int fifo_nonblock_set(int fd, int op);
extern int fifo_nonblock_on (int fd);
extern int fifo_nonblock_off(int fd);


/* File predicates 
``````````````````````````````````````````````````````````````````````````````*/
extern bool exists(const char *path);
extern int is_valid_fd(int fd);
extern int ftype(const char *path);
extern const char *sperm(__mode_t mode);


/* Filenames and pathnames 
``````````````````````````````````````````````````````````````````````````````*/
extern const char *scwd(void);
extern const char *curdir(void);

extern const char *getdirpath(DIR *dir);
extern bool is_relpath(const char *path);
extern void make_path_absolute(char *path);
extern const char *absolute_path(const char *path);

extern const char *gethome_uid(uid_t uid);
extern const char *gethome(void);

extern int tempname(char *templ);
extern const char *base_name(char *path);

extern void srename(const char *oldname, const char *newname);


extern bool file_exists(const char *path);
extern bool file_access(const char *path);
extern bool file_readable(const char *path);
extern bool file_writable(const char *path);


/* Current working directory tracking 
``````````````````````````````````````````````````````````````````````````````*/
struct cwd_t { 
        char home[PATHSIZE]; 
        char jump[PATHSIZE];
        bool away; 
};

extern void cwd_mark   (struct cwd_t *breadcrumb);
extern void cwd_shift  (struct cwd_t *breadcrumb, const char *path);
extern void cwd_revert (struct cwd_t *breadcrumb);
extern void cwd_setjump(struct cwd_t *breadcrumb, const char *path);
extern void cwd_jump   (struct cwd_t *breadcrumb);



/* Text file parsing 
``````````````````````````````````````````````````````````````````````````````*/
extern void get_tokenf(char *dst, char, char, char, const char *tok, const char *path);
extern char    *tokenf(char, char, char, const char *tok, const char *path);

/* Macros for most common scenario */
#define get_token(dst, tok, path) get_tokenf(dst, '\n', ' ', '#', tok, path)
#define token(tok, path)          tokenf('\n', ' ', '#', tok, path)


/* Pipes 
``````````````````````````````````````````````````````````````````````````````*/
extern int bounce(char *buf, size_t max, const char *fmt, ...);



/******************************************************************************
 * FILETYPE EXTENSIONS
 *
 * The following macros should be defined in <sys/stat.h>:
 *
 * #define S_IFMT  00170000   Mask the mode bytes describing file type
 * #define S_IFSOCK 0140000   Socket
 * #define S_IFLNK  0120000   Symlink
 * #define S_IFREG  0100000   Regular file
 * #define S_IFBLK  0060000   Block device
 * #define S_IFDIR  0040000   Directory
 * #define S_IFCHR  0020000   Character device
 * #define S_IFIFO  0010000   FIFO (named pipe)
 *
 * The same file should contain type predicates of the form:
 *
 * #define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
 * ...
 ******************************************************************************/

/*
 * Expands to the type value used in the mode quantity 'm'. 
 *
 * CAVEAT 
 * The result of this macro is NOT compatible as an argument to the 
 * S_ISxxx macros outlined above. Those perform the equivalent of 
 * F_TYPE(m) internally.
 */
#define F_TYPE(mode) ((mode) & S_IFMT)

/* 
 * Easier-to-read filetype names 
 */
#define F_PIPE  S_IFIFO
#define F_SOCK  S_IFSOCK
#define F_LINK  S_IFLNK
#define F_REG   S_IFREG
#define F_BLOCK S_IFBLK
#define F_CHAR  S_IFCHR
#define F_DIR   S_IFDIR

/* 
 * Sometimes we want to signal that we wish to test for a
 * hidden file, whatever the implementation may define that
 * as.
 */
//#define F_HID (0160000)   // Filter for hidden files


#endif

