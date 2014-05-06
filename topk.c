
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "topk.h"

static uint32_t leveldb_bloom_hash(unsigned char *b, size_t len);
static void bubble_sort(lelt_t * list, int idx);

topk_t *topk_new(int size)
{

    topk_t *tk = calloc(1, sizeof(topk_t));

    tk->size = size;

    tk->msize = 0;
    tk->mlist = calloc(size, sizeof(lelt_t));

    tk->hsize = size * 6;
    tk->htable = calloc(tk->hsize, sizeof(helt_t));

    return tk;
}

void topk_free(topk_t * tk)
{

    for (int i = 0; i < tk->msize; i++) {
	free(tk->mlist[i].v);
    }
    free(tk->mlist);

    free(tk->htable);

    free(tk);
}


void topk_insert(topk_t * tk, unsigned char *v, size_t vlen)
{

    uint32_t h = leveldb_bloom_hash(v, vlen) % tk->hsize;

    if (tk->htable[h].c) {

	// check our cached index first
	int i = tk->htable[h].idx;
	if (tk->mlist[i].vlen == vlen
	    && memcmp(v, tk->mlist[i].v, vlen) == 0) {
	    tk->mlist[i].count++;
	    bubble_sort(tk->mlist, i);
	    return;
	}
	// nope, full search of the list then
	for (i = 0; i < tk->msize; i++) {
	    if (tk->mlist[i].vlen == vlen
		&& memcmp(v, tk->mlist[i].v, vlen) == 0) {
		tk->mlist[i].count++;
		tk->htable[h].idx = i;
		bubble_sort(tk->mlist, i);
		return;
	    }
	}
    }
    // nope, not found :(, must be a hash collision

    // there's space to add it
    if (tk->msize < tk->size) {

	// but we can add it
	lelt_t *l = tk->mlist + tk->msize;
	l->v = malloc(vlen);
	memcpy(l->v, v, vlen);
	l->vlen = vlen;

	l->count = 1;
	l->err = 0;

	bubble_sort(tk->mlist, tk->msize);
	tk->msize++;

	// one more item tracked for this hash value
	tk->htable[h].c++;

	return;
    }
    // the 'minimum' currently tracked item is the last one, we keep the list sorted
    lelt_t *u = tk->mlist + tk->msize - 1;

    if (tk->htable[h].alpha + 1 < u->count) {
	// don't track it -- not popular enough
	tk->htable[h].alpha++;
	return;
    }
    // no space to add it, but let's replace the item in 'u'
    int h2 = leveldb_bloom_hash(u->v, u->vlen) % tk->hsize;
    tk->htable[h2].c--;
    tk->htable[h2].alpha = u->count;
    tk->htable[h].c++;

    free(u->v);
    u->v = malloc(vlen);
    memcpy(u->v, v, vlen);
    u->vlen = vlen;

    u->err = tk->htable[h].alpha;
    u->count = tk->htable[h].alpha + 1;

    bubble_sort(tk->mlist, tk->msize - 1);

    return;
}

// bubble sort the element at idx into its new position
// smallest item should be at the end of the list
int lelt_less(lelt_t * x, lelt_t * y)
{
    return (x->count < y->count) || (x->count == y->count
				     && x->err > y->err);
}

void bubble_sort(lelt_t * list, int idx)
{
    while (idx != 0 && lelt_less(list + idx - 1, list + idx)) {
	lelt_t tmp = list[idx];
	list[idx] = list[idx - 1];
	list[idx - 1] = tmp;
	idx--;
    }
}

// from leveldb, a murmur-lite
static uint32_t leveldb_bloom_hash(unsigned char *b, size_t len)
{
    const uint32_t seed = 0xbc9f1d34;
    const uint32_t m = 0xc6a4a793;

    uint32_t h = seed ^ len * m;
    while (len >= 4) {
	h += b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
	h *= m;
	h ^= h >> 16;
	b += 4;
	len -= 4;
    }

    switch (len) {
    case 3:
	h += b[2] << 16;
    case 2:
	h += b[1] << 8;
    case 1:
	h += b[0];
	h *= m;
	h ^= h >> 24;
    }
    return h;
}
