#include "rbspsc.h"
#include <stdlib.h>
#include <string.h>

static int is_pow2(size_t x){ return x && ((x & (x-1))==0); }
static inline size_t idx_of(const rbspsc_t* rb, size_t abs){
    return rb->pow2 ? (abs & rb->mask) : (abs % rb->capacity);
}

int rbspsc_init(rbspsc_t* rb, size_t capacity){
    if(!rb || capacity < 8) return -1;
    rb->buf = (uint8_t*)malloc(capacity);
    if(!rb->buf) return -2;
    rb->capacity = capacity;
    rb->pow2 = is_pow2(capacity);
    rb->mask = rb->pow2 ? (capacity - 1) : 0;
    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    return 0;
}

void rbspsc_free(rbspsc_t* rb){
    if(rb && rb->buf){ free(rb->buf); rb->buf=NULL; }
}

size_t rbspsc_push(rbspsc_t* rb, const void* data, size_t len){
    if(!rb || !rb->buf || !data || len==0) return 0;

    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);

    size_t room = rb->capacity - (head - tail);
    size_t n = (len < room) ? len : room;
    if(n==0) return 0;

    size_t hidx = idx_of(rb, head);
    size_t first = rb->capacity - hidx;
    if(first > n) first = n;
    memcpy(&rb->buf[hidx], data, first);

    size_t remain = n - first;
    if(remain){
        memcpy(&rb->buf[0], (const uint8_t*)data + first, remain);
    }

    atomic_store_explicit(&rb->head, head + n, memory_order_release);
    return n;
}

size_t rbspsc_pop(rbspsc_t* rb, void* out, size_t len){
    if(!rb || !rb->buf || !out || len==0) return 0;

    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);

    size_t avail = head - tail;
    size_t n = (len < avail) ? len : avail;
    if(n==0) return 0;

    size_t tidx = idx_of(rb, tail);
    size_t first = rb->capacity - tidx;
    if(first > n) first = n;
    memcpy(out, &rb->buf[tidx], first);

    size_t remain = n - first;
    if(remain){
        memcpy((uint8_t*)out + first, &rb->buf[0], remain);
    }

    atomic_store_explicit(&rb->tail, tail + n, memory_order_release);
    return n;
}
