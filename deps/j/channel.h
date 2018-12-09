#ifndef _CHANNEL_H
#define _CHANNEL_H
#include <stdlib.h>
#include <stdbool.h>


/******************************************************************************
 * MODES AND OPTIONS
 ******************************************************************************/
#define CH_ROLE(mode)   (((mode) & CH_PUB) == CH_PUB) ? CH_PUB : CH_SUB
#define CH_CREATE(mode) (((mode) & CH_NEW) == CH_NEW) ? true   : false

enum ch_role { CH_NEW = 0, CH_PUB = 1<<0, CH_SUB = 1<<1 };


/******************************************************************************
 * CHANNEL OBJECT 
 ******************************************************************************/
struct channel_t {
        enum ch_role role;
        int pub;
        int sub;
        int nub;
        int buf;
        int log;
};


/******************************************************************************
 * CHANNEL METHODS 
 ******************************************************************************/
extern void ch_open   (struct channel_t *ch, const char *path, int mode);
extern void ch_close  (struct channel_t *ch);
extern void ch_remove (const char *path);

extern size_t ch_read (struct channel_t *ch, void *dst, size_t max);
extern size_t ch_write(struct channel_t *ch, void *src, size_t nbytes);


#endif
