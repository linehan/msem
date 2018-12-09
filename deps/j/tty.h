#ifndef _JLIB_TTY_H 
#define _JLIB_TTY_H 

/* Extended character support (must precede #includes) */
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED 
#endif
#include <locale.h>
#include <wchar.h>
#include <ncurses.h>
#include <panel.h>

#if !defined(NCURSES_VERSION)
#error "Missing ncurses headers; you may need to install ncurses-devel"
#endif


#define BUFFER_INPUT            (1 << 0)
#define ECHO_INPUT              (1 << 1)
#define BLOCKING_INPUT          (1 << 2)
#define SHOW_CURSOR             (1 << 3)
#define DETECT_KEYPAD           (1 << 4)
#define USE_COLOR               (1 << 5)
#define USE_DEFAULT_COLORS      (1 << 6)
#define COLOR_PROGRAMMABLE      (1 << 7)
#define COLOR_256               (1 << 8)


int nc_set(bool value, int option);
int nc_start(void);
int nc_stop(void);
int nc_waitkey(void);
cchar_t *cch(wchar_t *wch, attr_t attr, short co);

void win_background(WINDOW *win, cchar_t *cch);
void win_background_set(WINDOW *win, cchar_t *cch);

#ifndef pan_refresh
#define pan_refresh()    \
        update_panels()
#endif

#ifndef scr_refresh
#define scr_refresh()    \
        update_panels(); \
        doupdate()
#endif

#ifndef win_refresh
#define win_refresh(win) \
        wrefresh(win)
#endif


/**
 * The ESC key code is somewhat implementation-defined, and ncurses 
 * doesn't define a macro for it as such, although it supports many 
 * other special keys. '27' is common enough for now, just don't forget!
 */
#ifndef KEY_ESC
#define KEY_ESC 27
#endif


#endif
