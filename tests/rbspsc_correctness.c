#include "rbspsc.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>

typedef struct {
    rbspsc_t* rb;
    size_t N;
    atomic_size_t produced, consumed;
    atomic_int error;
} ctx_t;

static void fill_pattern(uint8_t* dst, size_t off, size_t len){
    for(size_t i=0;i<len;i++) dst[i]=(uint8_t)((off+i)&0xFF);
}

static void* producer(void* arg){
    ctx_t* c=(ctx_t*)arg; uint8_t buf[4096];
    while(atomic_load(&c->produced)<c->N){
        size_t off=atomic_load(&c->produced);
        size_t chunk=sizeof(buf); if(chunk>c->N-off) chunk=c->N-off;
        fill_pattern(buf, off, chunk);
        size_t w=rbspsc_push(c->rb, buf, chunk);
        if(!w){ sched_yield(); continue; }
        atomic_fetch_add(&c->produced, w);
    }
    return NULL;
}
static void* consumer(void* arg){
    ctx_t* c=(ctx_t*)arg; uint8_t buf[4096], exp[4096];
    while(atomic_load(&c->consumed)<c->N){
        size_t off=atomic_load(&c->consumed);
        size_t r=rbspsc_pop(c->rb, buf, sizeof(buf));
        if(!r){ sched_yield(); continue; }
        fill_pattern(exp, off, r);
        if(memcmp(buf, exp, r)!=0){ fprintf(stderr,"Data mismatch @%zu\n", off); atomic_store(&c->error,1); break; }
        atomic_fetch_add(&c->consumed, r);
    }
    return NULL;
}

int main(void){
    rbspsc_t rb; if(rbspsc_init(&rb, 1<<16)!=0){ fprintf(stderr,"init failed\n"); return 1; }
    ctx_t ctx={ .rb=&rb, .N=10u*1024u*1024u, .produced=0, .consumed=0, .error=0 };
    pthread_t tp, tc; pthread_create(&tp,NULL,producer,&ctx); pthread_create(&tc,NULL,consumer,&ctx);
    pthread_join(tp,NULL); pthread_join(tc,NULL); rbspsc_free(&rb);
    if(atomic_load(&ctx.error)) return 1;
    if(ctx.produced!=ctx.consumed || ctx.produced!=ctx.N) return 1;
    puts("SPSC ring buffer correctness: OK"); return 0;
}
