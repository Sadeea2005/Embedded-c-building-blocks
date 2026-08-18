/*
 * ring_buffer.h  --  Fixed-size lock-free-ish circular buffer (SPSC)
 *
 * A ring buffer is THE canonical embedded data structure: it decouples a
 * fast producer (e.g. a UART receive interrupt) from a slower consumer
 * (your main loop) without any dynamic memory. Storage is a fixed array,
 * so there is no malloc and the memory footprint is known at compile time.
 *
 * Design notes worth saying out loud in an interview:
 *  - Capacity is kept as a power of two so that index wrap is a cheap AND
 *    (idx & (size-1)) instead of a modulo/branch.
 *  - We keep ONE slot empty to distinguish "full" from "empty" without an
 *    extra count variable -- head == tail means empty, head+1 == tail means
 *    full. This is the classic single-producer/single-consumer (SPSC) trick
 *    and needs no locking as long as one side only writes head and the other
 *    only writes tail (safe between an ISR and the main loop on an MCU).
 */
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint8_t  *buffer;   /* backing storage supplied by the caller           */
    uint32_t  size;     /* MUST be a power of two                           */
    volatile uint32_t head;  /* written by producer, read by consumer      */
    volatile uint32_t tail;  /* written by consumer, read by producer      */
} ring_buffer_t;

/* Initialise a ring buffer over caller-owned storage.
 * Returns false if size is 0 or not a power of two. */
bool rb_init(ring_buffer_t *rb, uint8_t *storage, uint32_t size);

/* Push one byte. Returns false if the buffer is full. O(1), never blocks. */
bool rb_put(ring_buffer_t *rb, uint8_t byte);

/* Pop one byte into *out. Returns false if the buffer is empty. O(1). */
bool rb_get(ring_buffer_t *rb, uint8_t *out);

bool     rb_is_empty(const ring_buffer_t *rb);
bool     rb_is_full(const ring_buffer_t *rb);
uint32_t rb_count(const ring_buffer_t *rb);   /* bytes currently stored */

#endif /* RING_BUFFER_H */
