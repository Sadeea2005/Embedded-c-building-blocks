#include "mem_pool.h"

bool mp_init(mem_pool_t *mp, void *storage,
             uint32_t block_size, uint32_t block_count) {
    if (mp == NULL || storage == NULL || block_count == 0 ||
        block_size < sizeof(void *)) {
        return false;
    }

    mp->pool        = (uint8_t *)storage;
    mp->block_size  = block_size;
    mp->block_count = block_count;
    mp->free_count  = block_count;

    /* Thread every block onto the free list. Each block's first bytes hold
     * the address of the next free block; the last block points to NULL. */
    mp->free_list = storage;
    uint8_t *cur = mp->pool;
    for (uint32_t i = 0; i < block_count - 1; ++i) {
        uint8_t *next = cur + block_size;
        *(void **)cur = next;      /* store "next" inside this free block */
        cur = next;
    }
    *(void **)cur = NULL;          /* terminate the list */
    return true;
}

void *mp_alloc(mem_pool_t *mp) {
    if (mp->free_list == NULL) {
        return NULL;               /* exhausted */
    }
    void *block   = mp->free_list;
    mp->free_list = *(void **)block;   /* pop head */
    mp->free_count--;
    return block;
}

void mp_free(mem_pool_t *mp, void *ptr) {
    if (ptr == NULL) {
        return;
    }
    *(void **)ptr = mp->free_list;     /* push onto head */
    mp->free_list = ptr;
    mp->free_count++;
}

uint32_t mp_free_count(const mem_pool_t *mp) {
    return mp->free_count;
}
