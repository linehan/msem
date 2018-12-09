#ifndef _TIMEUTIL_H
#define _TIMEUTIL_H
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>

#define SEC_IN_DAY       (86400)
#define NANO_IN_MILLI    (1000000)
#define SEC_IN_MS        (1000)
#define NS_TO_MS(n)      ((n)/NANO_IN_MILLI)
#define MS_TO_NS(m)      ((m)*NANO_IN_MILLI) 
#define MS_TO_US(m)      ((m)*1000) 

extern int  ms_since_midnight(void);
extern int sec_since_midnight(void);

extern bool sleep_ns(long nanoseconds);
extern bool sleep_ms(long milliseconds);

extern int clock_seconds(void);

extern long elapse_ms(long *counter);

#endif
