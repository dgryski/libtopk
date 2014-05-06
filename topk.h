/*  sketch element */

#include <stddef.h>

typedef struct {
    int alpha;
    int c;
    int idx;
} helt_t;

/* monitored list element */
typedef struct {
    unsigned char *v;
    size_t vlen;
    int count;
    int err;
} lelt_t;

typedef struct {
    int size;			/* number of elements to monitor -- maximum size of mlist */

    helt_t *htable;
    int hsize;			/* size of hash table */

    lelt_t *mlist;
    int msize;			/* current size of list */
} topk_t;


topk_t *topk_new(int size);
void topk_free(topk_t * tk);
void topk_insert(topk_t * tk, unsigned char *v, size_t vlen);
