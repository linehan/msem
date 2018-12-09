#ifndef _SIGNALS_H
#define _SIGNALS_H

#include <signal.h>

typedef void (*sig_handler_t)(int signo);
typedef void (*sig_cleanup_t)(void);

extern void sigreg(sig_cleanup_t cleanup);
extern void catch_signal(int signo);
extern void ignore(int signo);

#endif
