#ifndef _JLIB_INFO_H
#define _JLIB_INFO_H

extern int my_path(char *buf, size_t size);
extern int my_dirname(char *buf, size_t size);
extern int my_user(char *buf, size_t size);
extern int my_host(char *buf, size_t size);
extern int my_orig(char *buf, size_t size);
extern int my_time(char *buf, size_t size);

#endif
