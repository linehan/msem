#ifndef _INCLUDE_MSEM_H
#define _INCLUDE_MSEM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdbool.h>

int msem_create(char *path, char *tag, int init);
int msem_exists(char *path, char *tags);
int msem_open  (char *path, char *tag, int init);
int msem_remove(int semid);
int msem_close (int semid);

bool msem_scan(FILE *file, char *tag, key_t *key, int *semid);

int msem_query(int semid, char *query_code);
int msem      (int semid, char *mode, int timeout);


#endif
