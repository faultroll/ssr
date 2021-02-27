
#include "cfifo.h"
#include <stdint.h>
#include <stdlib.h>
#include "catomic.h"
#include "carray.h"

// https://www.cnblogs.com/shines77/p/4192787.html

typedef struct _cfifo_node_s_ cfifo_node_s;

struct _cfifo_node_s_ {
    void *value;
};
struct _cfifo_s_ {
    // int magic;
    // ATOMIC_VAR(bool) inited;
    ATOMIC_VAR(size_t) head, tail;
    ATOMIC_VAR(size_t) sentinel; // update after tail, avoiding w/w race
    ATOMIC_VAR(size_t) used;
    ARRAY_VAR(cfifo_node_s) buffer;
};

// #define CFIFO_MAGIC ('f' << 24 | 'i' << 16 | 'f' << 8 | 'o' << 0)

cfifo_s *cfifo_alloc(size_t size)
{
    size += 1; // for full & empty
    cfifo_s *fifo = malloc(sizeof(cfifo_s) + ARRAY_SIZE(cfifo_node_s, size));
    if (NULL == fifo)
        return fifo;
    ARRAY_BOUND(&fifo->buffer) = size;

    // fifo->magic = CFIFO_MAGIC;
    ATOMIC_VAR_STOR(&fifo->head, 0);
    ATOMIC_VAR_STOR(&fifo->tail, 0);
    ATOMIC_VAR_STOR(&fifo->sentinel, 0);
    ATOMIC_VAR_STOR(&fifo->used, 0);

    // ATOMIC_VAR_STOR(&fifo->inited, true);

    return fifo;
}

void cfifo_free(cfifo_s *fifo)
{
    free(fifo);
}

size_t cfifo_size(cfifo_s *fifo)
{
    return ATOMIC_VAR_LOAD(&fifo->used);
}

int cfifo_push(cfifo_s *fifo, void *value)
{
    // if (NULL == fifo || fifo->magic != CFIFO_MAGIC || !ATOMIC_VAR_LOAD(&fifo->inited))
    //     return;

    size_t size = ARRAY_BOUND(&fifo->buffer);
    size_t tail = 0, head = 0;
    do {
        tail = ATOMIC_VAR_LOAD(&fifo->tail);
        head = ATOMIC_VAR_LOAD(&fifo->head);
        if ((tail + 1) % size == head % size)
            // full
            return -1;
    } while (!ATOMIC_VAR_CAS(&fifo->tail, tail, tail + 1));
    // change value after tail update, avoiding w/w race
    ARRAY_ARRAY(&fifo->buffer)[tail % size].value = value;

    // then update sentinel to tell reader that writer is done
    size_t tick = 0;
    while (!ATOMIC_VAR_CAS(&fifo->sentinel, tail, tail + 1)) {
        if (++tick % 5000 == 0)
            // THREAD_YIELD
            ;
    }
    ATOMIC_VAR_FAA(&fifo->used, 1);

    return 0;
}

void *cfifo_pop(cfifo_s *fifo)
{
    // if (NULL == fifo || fifo->magic != CFIFO_MAGIC || !ATOMIC_VAR_LOAD(&fifo->inited))
    //     return;

    void *value = NULL;
    size_t size = ARRAY_BOUND(&fifo->buffer);
    size_t sentinel = 0, head = 0;
    do {
        sentinel = ATOMIC_VAR_LOAD(&fifo->sentinel);
        head = ATOMIC_VAR_LOAD(&fifo->head);
        if (head % size == sentinel % size)
            // empty
            return NULL;
        // change value before head update, avoiding r/w race
        value = ARRAY_ARRAY(&fifo->buffer)[head % size].value;
    } while (!ATOMIC_VAR_CAS(&fifo->head, head, head + 1));

    ATOMIC_VAR_FAA(&fifo->used, -1);

    return value;
}

