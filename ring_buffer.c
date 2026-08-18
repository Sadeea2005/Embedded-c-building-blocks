#include "ring_buffer.h"

static bool is_pow2(uint32_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

bool rb_init(ring_buffer_t *rb, uint8_t *storage, uint32_t size) {
    if (rb == NULL || storage == NULL || !is_pow2(size)) {
        return false;
    }
    rb->buffer = storage;
    rb->size   = size;
    rb->head   = 0;
    rb->tail   = 0;
    return true;
}

bool rb_is_empty(const ring_buffer_t *rb) {
    return rb->head == rb->tail;
}

bool rb_is_full(const ring_buffer_t *rb) {
    /* full when advancing head would collide with tail (one slot kept free) */
    return ((rb->head + 1) & (rb->size - 1)) == rb->tail;
}

uint32_t rb_count(const ring_buffer_t *rb) {
    return (rb->head - rb->tail) & (rb->size - 1);
}

bool rb_put(ring_buffer_t *rb, uint8_t byte) {
    uint32_t next = (rb->head + 1) & (rb->size - 1);
    if (next == rb->tail) {
        return false;               /* full */
    }
    rb->buffer[rb->head] = byte;
    rb->head = next;                /* publish AFTER the write */
    return true;
}

bool rb_get(ring_buffer_t *rb, uint8_t *out) {
    if (rb->head == rb->tail) {
        return false;               /* empty */
    }
    *out = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) & (rb->size - 1);
    return true;
}
