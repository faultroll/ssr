
enum {
    BUF_EMPTY,
    BUF_USED,
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
    uint32_t maxnum;
    uint32_t usednum;
    uint32_t maxsize;
    uint32_t recentusedidx;
};

bool init(uint32_t maxnum, uint32_t maxsize)
{
    uint64_t blockbase = sizeof(struct buffer) +
                         maxblocknum * sizeof(struct buffer_node);
    uint64_t totalmemsize = sizeof(struct buffer) +
                            maxblocknum * sizeof(struct buffer_node) +
                            maxnum * maxsize;
    m_pbufbase = new char[totalmemsize];
    // m_pbufbase->magic = ;
    m_pbufbase->maxnum = maxnum;
    m_pbufbase->usednum = 0;
    m_pbufbase->maxsize = maxsize;
    m_pbufbase->recentusedidx = 0;
    m_pstatebase = m_pbufbase + sizeof(struct buffer);
    for (uint32_t iblock = 0; iblock < maxnum; ++iblock) {
        m_pstatebase[iblock].offset = blockbase + iblock * maxsize;
        m_pstatebase[iblock].state = BUF_EMPTY;
        m_pstatebase[iblock].size = maxsize;
        m_pstatebase[iblock].refcount = 0;
    }
}

void *alloc(uint32_t size)
{
    return getbufaddr(regbufidx(size));
}

void free(void *ptr)
{
    unregbufidx(getbufidx(ptr));
}

int32_t regbufidx(uint32_t size)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;
    int id = pbuf->recentusedidx + 1, // TODO atomic_load
        max_idx = pbuf->maxnum;
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

int32_t unregbufidx(int32_t idx)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;
    int max_idx = pbuf->maxnum;
    if (id >= 0 && id < max_idx && (BUF_USED == pnode[id].state)) {
        if (ATOMIC_CAS(&pnode[id].state, BUF_USED, BUF_EMPTY)) {
            pnode[id].usedsize = 0;
            ATOMIC_FAA(&pbuf->usednum, -1);
            return id;
        }
    }

    return -1;
}

void *getbufaddr(int32_t idx)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;

    return (pbuf + pnode[idx].offset);
}

int32_t getbufidx(void *ptr)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;

    uint64_t offset = sizeof(struct buffer) +
                      pbuf->maxnum * sizeof(struct buffer_node);
    int32_t idx = (uint32_t)((char *)ptr - pbuf - offset) / pbuf->maxsize;

    return idx;
}

uint32_t getbufsize(int32_t idx)
{
    struct buffer *pbuf = (struct buffer *)m_pbufbase;
    struct buffer_node *pnode = (struct buffer_node *)m_pstatebase;

    return (pbuf + pnode[idx].size);
}
