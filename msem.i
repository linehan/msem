%module php_msem
%{
#include <stdbool.h>
%}

int msem_create(char *path, char *tag, int init);
int msem_open  (char *path, char *tag, int init);
int msem_remove(int semid);
int msem_close (int semid);

int msem_query(int semid, char *query_code);
int msem      (int semid, char *mode, int timeout=-1);

