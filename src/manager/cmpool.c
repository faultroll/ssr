
#include "cmpool.h"
#include <stdint.h>
#include <stdlib.h>
#include "catomic.h"

// TODO use same as lifo
enum {
    NST_EMPTY,
    NST_USED,
};

typedef struct _cmpool_node_s_ {
    ptrdiff_t offset;
    ATOMIC_VAR(int) state;
    ATOMIC_VAR(int) refcount;
    // size_t size;
    // size_t usedsize;
} cmpool_node_s;

typedef struct _cmpool_head_s_ {
    ATOMIC_VAR(int) magic;
    size_t blksz;
    ATOMIC_VAR(int) latestidx;
    // size_t usednum;
    ptrdiff_t blockbase;
    size_t blknum;
    cmpool_node_s index[];
} cmpool_head_s;

typedef struct _cmpool_s_ {
    void *mbase;
    size_t msize;
} cmpool_s;

#define CMPOOL_MAGIC ('m' << 24 | 'p' << 16 | 'l' << 8 | '\0' << 0)

cmpool_s *cmpool_init(void *mbase, size_t msize, size_t blksz)
{
    cmpool_s *mpool = malloc(sizeof(cmpool_s));
    mpool->mbase = mbase;
    mpool->msize = msize;

    cmpool_head_s *head = (cmpool_head_s *)mpool->mbase;
    if ((int dirty = ATOMIC_VAR_LOAD(&head->magic)) != CMPOOL_MAGIC) {
        // TODO may race with alloc
        head->blksz = blksz;
        ATOMIC_VAR_STOR(&head->latestidx, 0);
        head->blknum = (msize - sizeof(cmpool_head_s)) /
                       (blksz + sizeof(cmpool_node_s));
        head->blockbase = sizeof(cmpool_head_s) +
                          head->blknum * sizeof(cmpool_node_s);
        for (size_t i = 0; i < head->blknum; ++i) {
            head->index[i].offset = blockbase + i * head->blksz;
            ATOMIC_VAR_STOR(&head->index[i].state, NST_EMPTY);
            ATOMIC_VAR_STOR(&head->index[i].refcount, 0);
            // head->index[i].size = blksz;
        }
        if (!ATOMIC_VAR_CAS(&head->magic, dirty, CMPOOL_MAGIC)) {
            // TODO will failed when changed from dirty to another value
            /* return mpool; */
        } /* else {
            return mpool;
        } */
    } /* else {
        return mpool;
    } */

    return mpool;
}

void cmpool_fini(cmpool_s *mpool)
{
    free(mpool);
}

static int regbufidx(cmpool_head_s *head, size_t size)
{
    cmpool_head_s *pbuf = head;
    cmpool_node_s *pnode = pbuf->index;
    int idx = ATOMIC_VAR_LOAD(&pbuf->latestidx) + 1,
        max_idx = pbuf->blknum;
    if (/* idx >= 0 && */ idx < max_idx) {
        if (ATOMIC_CAS(&pnode[idx].state, NST_EMPTY, NST_USED)) {
            // pnode[idx].usedsize = 0;
            ATOMIC_VAR_STOR(&pbuf->latestidx, idx);
            // ATOMIC_FAA(&pbuf->usednum, 1);
            return idx;
        }
    }

    for (idx = 0; idx < max_id; ++idx) {
        if (ATOMIC_CAS(&pnode[idx].state, NST_EMPTY, NST_USED)) {
            // pnode[idx].usedsize = 0;
            ATOMIC_VAR_STOR(&pbuf->latestidx, idx);
            // ATOMIC_FAA(&pbuf->usednum, 1);
            return idx;
        }
    }

    return -1;
}

static void unregbufidx(cmpool_head_s *head, int idx)
{
    cmpool_head_s *pbuf = head;
    cmpool_node_s *pnode = pbuf->index;
    int max_idx = pbuf->blknum;
    if (idx >= 0 && idx < max_idx) {
        if (ATOMIC_CAS(&pnode[idx].state, NST_USED, NST_EMPTY)) {
            // pnode[idx].usedsize = 0;
            // ATOMIC_FAA(&pbuf->usednum, -1);
        }
    }
}

static void *getbufaddr(cmpool_head_s *head, int idx)
{
    cmpool_head_s *pbuf = head;
    cmpool_node_s *pnode = pbuf->index;
    int max_idx = pbuf->blknum;

    if (idx >= 0 && idx < max_idx)
        return (void *)((intptr_t)pbuf + pnode[idx].offset);
    else
        return NULL;
}

static int getbufidx(cmpool_head_s *head, void *ptr)
{
    cmpool_head_s *pbuf = head;
    cmpool_node_s *pnode = pbuf->index;

    return (int)(((intptr_t)ptr - (intptr_t)pbuf - pbuf->blockbase) / pbuf->blksz);
}

void *cmpool_alloc(cmpool_s *mpool, size_t size)
{
    return getbufaddr(mpool->mbase, regbufidx(mpool->mbase, size));
}

void cmpool_free(cmpool_s *mpool, void *ptr)
{
    unregbufidx(mpool->mbase, getbufidx(mpool->mbase, ptr));
}
