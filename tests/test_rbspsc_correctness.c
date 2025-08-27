#include "rbspsc.h"
#include <threads.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

typedef struct { rbspsc_t* rb; size_t N; atomic_ulong checksum; } ctx_t;

int producer(void* arg){
    ctx_t* c = (ctx_t*)arg;
    uint8_t buf[4096];
    size_t produced = 0;
    while(produced < c->N){
        size_t chunk = sizeof(buf);
        if(chunk > (c->N - produced)) chunk = c->N - produced;
        for(size_t i=0;i<chunk;i++) buf[i] = (uint8_t)((produced+i) & 0xFF);
        size_t w = 0;
        while(w == 0) w = rbspsc_push(c->rb, buf, chunk);
        for(size_t i=0;i<w;i++) atomic_fetch_add(&c->checksum, buf[i]);
        produced += w;
    }
    return 0;
}

int consumer(void* arg){
    ctx_t* c = (ctx_t*)arg;
    uint8_t buf[4096];
    size_t consumed = 0;
    unsigned long sum = 0;
    while(consumed < c->N){
        size_t r = rbspsc_pop(c->rb, buf, sizeof(buf));
        for(size_t i=0;i<r;i++) sum += buf[i];
        consumed += r;
    }
    unsigned long prod = atomic_load(&c->checksum);
    if(sum != prod){
        fprintf(stderr, "Checksum mismatch: cons=%lu prod=%lu\n", sum, prod);
        return 1;
    }
    return 0;
}

int main(){
    rbspsc_t rb;
    if(rbspsc_init(&rb, 1<<16) != 0){ fprintf(stderr,"init failed\n"); return 1; }
    ctx_t ctx = { .rb=&rb, .N=10u*1024u*1024u, .checksum=0 }; // 10 MB

    thrd_t tp, tc;
    thrd_create(&tp, producer, &ctx);
    thrd_create(&tc, consumer, &ctx);

    int rp, rc;
    thrd_join(tp, &rp);
    thrd_join(tc, &rc);

    rbspsc_free(&rb);
    if(rp!=0 || rc!=0){ fprintf(stderr,"threads failed\n"); return 1; }
    puts("SPSC ring buffer correctness: OK");
    return 0;
}
