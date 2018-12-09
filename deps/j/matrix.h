#ifndef __MATRIX_H
#define __MATRIX_H
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * DATA STRUCTURES 
 ******************************************************************************/

union mx_cell {
        uint32_t u;
        double   d;
};

struct mx_c {
        union mx_cell *val;
        int i;
        int y;
        int x;  
};

struct mx_t {
        int  l;
        int  h;
        int  w;
        char t;
        union mx_cell *val;
        union mx_cell *mem;
};



static union mx_cell DOPE;


/******************************************************************************
 * INTERFACE 
 ******************************************************************************/

int mx_create(struct mx_t *matrix, char type, int h, int w);
int mx_remove(struct mx_t *matrix);


/**
 * MX_NODOPE()
 * ```````````
 * Test whether an address is that of the dope cell
 * @ptr: address 
 */
#define MX_NODOPE(p) ((p) != &DOPE)


/**
 * MX_OFFSET()
 * ```````````
 * Use pointer arithmetic to determine the offset of an address.
 * @m: matrix struct 
 * @p: an address somewhere in @m
 */
#define MX_OFFSET(m, p) ((p)-((m)->mem))

/**
 * MX_ROW()
 * ````````
 * Determine the row of a matrix cell.
 * @m: matrix struct
 * @p: an address somewhere in @m 
 */
#define MX_ROW(m, p) (MX_OFFSET((m),(p)) / (m)->w)

/**
 * MX_COL()
 * ````````
 * Determine the column of a matrix cell.
 * @m: matrix struct
 * @p: an address somewhere in @m 
 */
#define MX_COL(m, p) (MX_OFFSET((m),(p)) % (m)->w)


#define mx_addr(m, y, x)        ((m)->mem + (y * (m)->w) + x)


/**
 * mx_val()
 * ````````
 * Expand to the value of a matrix cell
 *
 * @m: matrix struct
 * @y: row index 
 * @x: column index
 */
#define mx_val(m, y, x)         *mx_addr(m,y,x)


/**
 * mx_get()
 * ````````
 * Expand to the address of a matrix cell
 *
 * @m: matrix struct
 * @y: row index
 * @x: column index
 */
#define mx_get(m, y, x)         mx_addr(m,y,x)

/**
 * mx_set()
 * ````````
 * Assign a value to a matrix cell
 *
 * @m: matrix struct
 * @y: row index
 * @x: column index
 * @v: value to assign
 */
//#define mx_set(m, y, x, v)   (*(mx_get((m),(y),(x))) = (v))


/******************************************************************************
 * NEIGHBORHOOD ACCESS 
 *
 * von Neumann neighborhood                     N
 *                                              
 * The four cells which orthogonally         W  c  E
 * surround a central cell on a two-            
 * dimensional square lattice.                  S
 *
 *
 * Moore neighborhood                        NW N NE
 *                      
 * The eight cells which surround a          W  c  E
 * central cell on a two-dimensional 
 * square lattice.                           SW S SE
 * 
 ******************************************************************************/

/*

        mx_set(&mx, val, 0, 0, 0,
                         2, 5, 1,
                         8, 3, 2);

 also, combine mx and val into one pointer, so that you
 can just use val and then use container_of and offset_of
 to cast back to the enclosing matrix structure like so

        mx_cell(mxc, 0, 0, 0,
                     2, 5, 1,
                     8, 3, 2);

*/
#define mx_yx mx_get

#define mx_set(nw, n, ne, w, c, e, sw, s, se, m, p)     \
        do {                                            \
                if ((m)->t == 'u') {                    \
                        p->u = c;                       \
                        mx_n(m, p)->u = n;              \
                        mx_w(m, p)->u = w;              \
                        mx_e(m, p)->u = e;              \
                        mx_s(m, p)->u = s;              \
                        mx_nw(m, p)->u = nw;            \
                        mx_ne(m, p)->u = ne;            \
                        mx_sw(m, p)->u = sw;            \
                        mx_se(m, p)->u = se;            \
                } else {                                \
                        p->d = c;                       \
                        mx_n(m, p)->d = n;              \
                        mx_w(m, p)->d = w;              \
                        mx_e(m, p)->d = e;              \
                        mx_s(m, p)->d = s;              \
                        mx_nw(m, p)->d = nw;            \
                        mx_ne(m, p)->d = ne;            \
                        mx_sw(m, p)->d = sw;            \
                        mx_se(m, p)->d = se;            \
                }                                       \
        } while (0)

/*
 * Get addr of neighborhood cell relative to central addr 
 *
 * @m: matrix struct
 * @p: address of central cell
 *
 * NOTE
 * If the neighborhood location is out-of-bounds,
 * the address of a 'dope' cell is returned.
 *
 * This allows the caller to assign values that would
 * be meaningless to an actual location in memory,
 * sparing the caller having to test and branch on
 * boundary cases.
 */

#define mx_n(m, p)                                                 \
        ((MX_NODOPE(p) && MX_ROW(m, p) > 1)                        \
                ? (union mx_cell *)((p) - (m)->w)                  \
                : (union mx_cell *)&DOPE)

#define mx_s(m, p)                                                 \
        ((MX_NODOPE(p) && MX_ROW(m, p) < ((m)->h))                 \
                ? (union mx_cell *)((p) + (m)->w)                  \
                : (union mx_cell *)&DOPE)

#define mx_w(m, p)                                                 \
        ((MX_NODOPE(p) && MX_COL(m, p) > 1)                        \
                ? (union mx_cell *)((p)-1)                         \
                : (union mx_cell *)&DOPE)

#define mx_e(m, p)                                                 \
        ((MX_NODOPE(p) && MX_COL(m, p) < (m)->w-1)                 \
                ? (union mx_cell *)((p)+1)                         \
                : (union mx_cell *)&DOPE)

#define mx_nw(m, p)                                                \
        (mx_w(m, mx_n(m, p)))

#define mx_ne(m, p)                                                \
        (mx_e(m, mx_n(m, p)))

#define mx_sw(m, p)                                                \
        (mx_w(m, mx_s(m, p)))

#define mx_se(m, p)                                                \
        (mx_e(m, mx_s(m, p)))



/******************************************************************************
 * ITERATORS 
 ******************************************************************************/

/**
 * mx_foreach()
 * ````````````
 * Iterate through the matrix in raster order
 *
 * @m: mx_t struct
 * @c: mx_c struct, updated on each loop 
 */
#define __mx_init(m, c)                                 \
        (c)->val = (m)->mem,                            \
        (c)->i = (c)->y = (c)->x = 0

#define __mx_test(m, c)                                 \
        (c)->i < (m)->l

#define __mx_next(m, c)                                 \
        ((c)->val)++,                                   \
        ((c)->i)++,                                     \
        (!((c)->x = (((c)->x+1) % (m)->w)) && (c)->y++) 

#define mx_foreach(c, m)                                \
        for (__mx_init((m), (c));                       \
             __mx_test((m), (c));                       \
             __mx_next((m), (c)))


#endif
