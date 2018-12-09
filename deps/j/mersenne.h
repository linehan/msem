#ifndef _LIBJDL_MATH_MERSENNE_H
#define _LIBJDL_MATH_MERSENNE_H

extern void          init_mersenne(void);
extern void          mt_check     (void);
extern unsigned long mt_random    (void);
extern double        mt_normalize (void);
#define mt_random    mt_normalize

#endif
