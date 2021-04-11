
#include "cmpool.h"
#include <stdint.h>
#include <stdlib.h>
#include "catomic.h"

// TODO use one as lifo
enum {
    NST_EMPTY,
    NST_USED,
};

struct buffer_node {
    uint64_t offset;
    uint32_t state;
    uint32_t size;
    uint32_t usedsize;
    uint32_t refcount;
}

struct buffer {
    uint64_t magic;
    uint32_t blknum;
    uint32_t usednum;
    uint32_t blksz;
    uint32_t recentusedidx;
};

cmpool_s *cmpool_init(size_t blknum, size_t blksz)
{
    uint64_t blockbase = sizeof(struct buffer) +
                         maxblocknum * sizeof(struct buffer_node);
    uint64_t totalmemsize = sizeof(struct buffer) +
                            maxblocknum * sizeof(struct buffer_node) +
                            blknum * blksz;
    m_pbufbase = malloc(totalmemsize);
    // m_pbufbase->magic = ;
    m_pbufbase->blknum = blknum;
    m_pbufbase->usednum = 0;
    m_pbufbase->blksz = blksz;
    m_pbufbase->recentusedidx = 0;
    m_pstatebase = m_pbufbase + sizeof(struct buffer);
    for (uint32_t iblock = 0; iblock < blknum; ++iblock) {
        m_pstatebase[iblock].offset = blockbase + iblock * blksz;
        m_pstatebase[iblock].state = BUF_EMPTY;
        m_pstatebase[iblock].size = blksz;
        m_pstatebase[iblock].refcount = 0;
    }
}

void cmpool_fini(cmpool_s *mpool)
{
    free(m_pbufbase);
}

void *cmpool_alloc(cmpool_s *mpool, size_t size)
{
    return getbufaddr(regbufidx(size));
}

void cmpool_free(cmpool_s *mpool, void *ptr)
{
    unregbufidx(getbufidx(ptr));
}

static int32_t regbufidx(uint32_t size)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;
    int id = pbuf->recentusedidx + 1, // TODO atomic_load
        max_idx = pbuf->blknum;
    if (id >= 0 && id < max_idx && (BUF_EMPTY == pnode[id].state)) {
        if (ATOMIC_CAS(&pnode[id].state, BUF_EMPTY, BUF_USED)) {
            pnode[id].usedsize = 0;
            pbuf->recentusedidx = id; // TODO atomic_store needed
            ATOMIC_FAA(&pbuf->usednum, 1);
            return id;
        }
    }

    for (id = 0; id < max_id; ++id) {
        if (pnode[id].state != BUF_EMPTY)
            continue;
        if (ATOMIC_CAS(&pnode[id].state, BUF_EMPTY, BUF_USED)) {
            pnode[id].usedsize = 0;
            pbuf->recentusedidx = id; // TODO atomic_store needed
            ATOMIC_FAA(&pbuf->usednum, 1);
            return id;
        }
    }

    return -1;
}

static int32_t unregbufidx(int32_t idx)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;
    int max_idx = pbuf->blknum;
    if (id >= 0 && id < max_idx && (BUF_USED == pnode[id].state)) {
        if (ATOMIC_CAS(&pnode[id].state, BUF_USED, BUF_EMPTY)) {
            pnode[id].usedsize = 0;
            ATOMIC_FAA(&pbuf->usednum, -1);
            return id;
        }
    }

    return -1;
}

static void *getbufaddr(int32_t idx)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;

    return (pbuf + pnode[idx].offset);
}

static int32_t getbufidx(void *ptr)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;

    uint64_t offset = sizeof(struct buffer) +
                      pbuf->blknum * sizeof(struct buffer_node);
    int32_t idx = (uint32_t)((char *)ptr - pbuf - offset) / pbuf->blksz;

    return idx;
}

static uint32_t getbufsize(int32_t idx)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;

    return (pbuf + pnode[idx].size);
}
