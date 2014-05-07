
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "topk.h"

#define MAXLINELEN  256

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("usage: %s testdata/domains.txt\n", argv[0]);
        exit(1);
    }

    unsigned char line[MAXLINELEN];
    FILE *f = fopen(argv[1], "r");
    topk_t *tk = topk_new(100);

    while(fgets((char *)line, MAXLINELEN, f)) {
        size_t len = strlen((char *)line);
        line[len-1] = '\0';
        topk_insert(tk, line, len);
    }

    for(int i=0;i<tk->msize;i++) {
        printf("%s %d\n", tk->mlist[i].v, tk->mlist[i].count);
    }

//    printf("cache_hits=%d total_lookups=%d success=%f\n", tk->cache_hits, tk->total_lookups, (double)(tk->cache_hits)/(double)(tk->total_lookups));

    topk_free(tk);
}
