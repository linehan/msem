#ifndef __TEXTUTILS_H
#define __TEXTUTILS_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <stdbool.h>

/* Initialization ----------------------------------------------------------- */
extern void szero(char *str);

extern int ccount(char *str, int c);

/* Safe strings ------------------------------------------------------------- */
extern char  *sdup(const char *str);
extern char  *sldup(const char *str, size_t max);
extern size_t slcpy(char *dst, const char *src, size_t siz);
extern size_t slcat(char *dst, const char *src, size_t siz);
extern size_t sbif(char *l, char *r, const char *str, const char *tok);
extern int bifurcate(const char *str, const char *tok, char *l, char *r);
extern size_t concat(char *dst, size_t max, const char *str, ...);
#define strbif    sbif

/* String sets -------------------------------------------------------------- */
extern size_t catenate(char *dest, size_t max, int n, char *strings[]);
//const char *concat(const char *a, const char *b);
extern char *match(const char *haystack, const char *needle);
extern char *field(const char *string, const char *delimiter);
extern int    ntok(const char *str, const char *tok);
extern void chrswp(char *src, char at, char with, size_t len);

//extern ssize_t getstr(char **buf, size_t *len, FILE *stream);

/* Format print ------------------------------------------------------------- */
//extern size_t pumpf(char **strp, const char *fmt, ...);
//extern size_t vpumpf(char **strp, const char *fmt, va_list args);

/* Whitespace --------------------------------------------------------------- */
extern size_t trimcpy(char *dst, const char *src);
extern char *trimws(char *str);

extern bool is_ws(char c);

extern void strip_comments(char *str);

extern char *tail(char *string);

extern char *bin_to_ascii(int c, int use_hex);
extern void esc_fputs(char *str, size_t max, FILE *stream);

extern void argv_print(int argc, char *argv[]);

#define SJOIN_CHUNKSIZE 1024

#define sjoin(dst, str)         sljoin((dst), (str), SJOIN_CHUNKSIZE)
#define sjoinf(dst, fmt, ...)   sljoinf((dst), SJOIN_CHUNKSIZE, fmt, __VA_ARGS__)

char *sljoin(char *dst, const char *str, size_t chunk_size);
char *sljoinf(char *dst, size_t chunk_size, const char *fmt, ...);

char *from_file(char *path);



/* Raw memory --------------------------------------------------------------- */

extern char *memseq(void *dst, const void *src, size_t len);

extern void *textutils_memmem(const void *haystack, const void *needle);
extern char *textutils_strstr(const char *haystack, const char *needle);
extern void *textutils_memchr(const void *src_void, int c, size_t len);

/* Nice macros -------------------------------------------------------------- */

#define STRCMP(a,b) (strcmp((a),(b)) == 0) ? true : false
#define isarg(n, string) (STRCMP(argv[(n)], (string)))
#define ARG(n)           (argv[(n)])

#define STREMPTY(s) (STRCMP((s),""))


#define IS_HEXDIGIT(x) (isdigit(x)||('a'<=(x)&&(x)<='f')||('A'<=(x)&&(x)<='F'))
#define IS_OCTDIGIT(x) ('0'<=(x) && (x)<='7')
extern int hex2bin(int c);
extern int oct2bin(int c);
extern int esc(char **s);

extern bool strcomb(char *haystack, char *needle);

extern wchar_t *wcs_wrap(wchar_t *buf, wchar_t *wcs, int line_width);


/**
 * concat
 * ``````
 * Return pointer to a static value of 2 concatenated strings.
 * @a: first string (head)
 * @b: second string (tail)
 * Return: pointer to the concateneated string, static. 
 */
static inline const char *join2(const char *a, const char *b)
{
        #define BIG 9000
        static char buffer[BIG];

        slcpy(buffer, a, BIG);
        slcat(buffer, b, BIG);

        return buffer;
}

#endif
