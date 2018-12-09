#ifndef __BINHEAP_H
#define __BINHEAP_H
#include <stdint.h>
#include <stdbool.h>


#define leftof(i)   ((i)<<1)
#define rightof(i)  (((i)<<1)+1)
#define parentof(i) ((i)>>1)


#define HEAP_ROOT     1   /* The index of the root node */
#define HEAP_MAX      4096
#define HEAP_MAX_PRI  999  /* The maximum admissible priority of a node */



/******************************************************************************
 * ABSTRACT DATA TYPES
 ******************************************************************************/

/*
 * Tree node encapsulation
 */
struct heap_node {
        uint32_t key;  /* Identifier */
        double pri;    /* Priority */
        void *data;    /* User pointer */
};

/*
 * Binary heap object
 */
struct heap_t {
        int32_t max;                /* Maximum number of nodes in the tree */
        int32_t n;                  /* Current number of nodes in the tree */
        struct heap_node **node;
};


//struct bheap_t *new_bh(uint32_t maxsize);
//int             del_bh(struct bheap_t **bh);


struct heap_t  *heap_create  (uint32_t maxsize);
int             heap_destroy (struct heap_t **h);
void            heap_truncate(struct heap_t *h);

bool  heap_add (struct heap_t *h, double pri, uint32_t key, void *x);
void *heap_pop (struct heap_t *h);
void *heap_peek(struct heap_t *h, int i);

bool heap_index_valid(struct heap_t *h, int i);
bool heap_is_empty   (struct heap_t *h);
bool heap_is_full    (struct heap_t *h);
bool heap_has_member (struct heap_t *h, uint32_t key);

bool     heap_node_setpri(struct heap_t *h, double pri, uint32_t key);
double   heap_priority_at(struct heap_t *h, int i);
uint32_t heap_key_at(struct heap_t *h, int i);







//void     bh_empty     (struct bheap_t *bh);
//bool     bh_valid     (struct bheap_t *bh, int i);
//bool     bh_is_empty  (struct bheap_t *bh);
//bool     bh_is_full   (struct bheap_t *bh);
//bool     bh_has_member(struct bheap_t *bh, uint32_t key);
//int      bh_primax    (struct bheap_t *bh, int a, int b);
//void     bh_swap      (struct bheap_t *bh, int a, int b);
//int      bh_bigchild  (struct bheap_t *bh, int node);
//void     bh_siftup    (struct bheap_t *bh, int node);
//void     bh_siftdown  (struct bheap_t *bh, int node);
//void     heapify      (struct bheap_t *bh);
//bool     bh_add       (struct bheap_t *bh, double pri, uint32_t key, void *x);
//void    *bh_pop       (struct bheap_t *bh);
//void    *bh_peek      (struct bheap_t *bh, int i);
//bool     bh_setpri    (struct bheap_t *bh, double pri, uint32_t key);
//double   bh_getpri    (struct bheap_t *bh, int i);
//uint32_t bh_getkey    (struct bheap_t *bh, int i);

#endif
