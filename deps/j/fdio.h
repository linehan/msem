#ifndef _FD_IO_H
#define _FD_IO_H
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

extern size_t fd_read(int fd, void *dst, size_t nbytes);
extern size_t fd_write(int fd, void *src, size_t nbytes);

extern int fd_getc(int fd);
extern int fd_putc(int c, int fd);

extern size_t fd_readb (void *dst, size_t bs, size_t nb, register int fd);
extern size_t fd_writeb(const void *src, size_t bs, size_t nb, register int fd);

extern size_t fd_readloop(int fd, void *dst, size_t nbytes, volatile int *loop);

extern size_t fd_readline(int fd, char *dst, size_t nbytes);
extern size_t fd_read_tok(int fd, char *dst, size_t nbytes, int tok);

extern size_t fd_dump(int fd_src, char *buf_dst, size_t maxbytes);
extern size_t fd_print(int fd_src, FILE *fs_dst);

extern size_t fd_copy(int fd_src, int fd_dst);

extern bool   fd_poll(int fd, long int seconds, long int microseconds);
extern int isready(int fd);
extern size_t fd_spool(int fd, FILE *stream);

extern int fd_setpos(int fd, int ofs, int whence);
extern int fd_getpos(int fd);

extern size_t copy_fd(int dst, int src);

#endif
