// Lock-free Single-Producer/Single-Consumer ring buffer (byte-based)
// C11 atomics; recomandat capacity putere a lui 2 (dar merge și altfel).
#pragma once
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t*      buf;
    size_t        capacity;     // mărimea bufferului
    atomic_size_t head;         // index absolut de scriere (producer)
    atomic_size_t tail;         // index absolut de citire (consumer)
    size_t        mask;         // capacity-1 dacă e putere a lui 2
    int           pow2;         // 1 dacă e putere a lui 2 (optimizăm modulo)
} rbspsc_t;

int    rbspsc_init(rbspsc_t* rb, size_t capacity);
void   rbspsc_free(rbspsc_t* rb);
size_t rbspsc_push(rbspsc_t* rb, const void* data, size_t len);
size_t rbspsc_pop (rbspsc_t* rb,       void* data, size_t len);

// Helpers
static inline size_t rbspsc_size(const rbspsc_t* rb) {
    size_t h = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t t = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return h - t;
}
static inline size_t rbspsc_capacity(const rbspsc_t* rb){ return rb->capacity; }
static inline size_t rbspsc_space(const rbspsc_t* rb){ return rb->capacity - rbspsc_size(rb); }
