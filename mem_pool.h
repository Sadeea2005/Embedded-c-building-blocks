/*
 * mem_pool.h  --  Fixed-block-size memory allocator (a.k.a. block pool)
 *
 * Firmware usually BANS malloc/free: heap fragmentation over a device's
 * multi-year uptime is a real failure mode, and malloc's timing is
 * non-deterministic. The standard replacement is a pool of equally-sized
 * blocks carved from one static array. Allocation and free are O(1) and
 * never fragment, because every block is interchangeable.
 *
 * Implementation: an intrusive free list. Each free block stores, in its
 * own memory, a pointer to the next free block -- so the "list" costs zero
 * extra bytes. mp_alloc pops the head; mp_free pushes back onto the head.
 */
#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint8_t *pool;         /* caller-owned backing storage           */
    void    *free_list;    /* head of the singly-linked free list    */
    uint32_t block_size;   /* bytes per block (>= sizeof(void*))     */
    uint32_t block_count;  /* total blocks                           */
    uint32_t free_count;   /* blocks currently free (for diagnostics)*/
} mem_pool_t;

/* Carve `storage` (block_size * block_count bytes) into a free list.
 * block_size must be >= sizeof(void*); returns false otherwise. */
bool  mp_init(mem_pool_t *mp, void *storage,
              uint32_t block_size, uint32_t block_count);

/* Return a pointer to one free block, or NULL if the pool is exhausted. */
void *mp_alloc(mem_pool_t *mp);

/* Return a block to the pool. `ptr` must have come from mp_alloc. */
void  mp_free(mem_pool_t *mp, void *ptr);

uint32_t mp_free_count(const mem_pool_t *mp);

#endif /* MEM_POOL_H */
