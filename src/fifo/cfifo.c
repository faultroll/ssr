
#include "cfifo.h"
#include <stdint.h>
#include <stdlib.h>
#include "catomic.h"

typedef struct _cfifo_node_s_ cfifo_node_s;

struct _cfifo_node_s_ {
    void *value;
};
struct _cfifo_s_ {
    int magic;
    // ATOMIC_VAR(bool) inited;
    cfifo_node_s *buffer;
    ATOMIC_VAR(size_t) head, tail;
    ATOMIC_VAR(size_t) max_read_head; // sentinel
    size_t size;
    ATOMIC_VAR(size_t) used;
};

#define CFIFO_MAGIC ('f' << 24 | 'i' << 16 | 'f' << 8 | 'o' << 0)

cfifo_s *cfifo_alloc(size_t size)
{
    cfifo_s *fifo = malloc(sizeof(cfifo_s));
    if (NULL == fifo)
        return fifo;

    fifo->buffer = malloc(size * sizeof(cfifo_node_s));
    if (fifo->buffer == NULL) {
        free(fifo);
        fifo = NULL;
        return fifo;
    }

    fifo->magic = CFIFO_MAGIC;
    ATOMIC_VAR_STOR(&fifo->head, 0);
    ATOMIC_VAR_STOR(&fifo->tail, 0);
    ATOMIC_VAR_STOR(&fifo->max_read_head, 0);
    fifo->size = size + 1; // for full & empty
    ATOMIC_VAR_STOR(&fifo->used, 0);

    // ATOMIC_VAR_STOR(&fifo->inited, true);

    return fifo;
}

void cfifo_free(cfifo_s *fifo)
{
    free(fifo->buffer);
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

    size_t tail = 0, head = 0;
    do {
        tail = ATOMIC_VAR_LOAD(&fifo->tail);
        head = ATOMIC_VAR_LOAD(&fifo->head);
        if ((tail + 1) % fifo->size == head % fifo->size)
            // full
            return -1;
    } while (!ATOMIC_VAR_CAS(&fifo->tail, &tail, tail + 1));

    fifo->buffer[tail % fifo->size].value = value;
    size_t tick = 0;
    while (!ATOMIC_VAR_CAS(&fifo->max_read_head, &tail, tail + 1)) {
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
    size_t max_read_head = 0, head = 0;
    do {
        max_read_head = ATOMIC_VAR_LOAD(&fifo->max_read_head);
        head = ATOMIC_VAR_LOAD(&fifo->head);
        if (head % fifo->size == max_read_head % fifo->size)
            // empty
            return NULL;

        value = fifo->buffer[head % fifo->size].value;
    } while (!ATOMIC_VAR_CAS(&fifo->head, &head, head + 1));

    ATOMIC_VAR_FAA(&fifo->used, -1);

    return value;
}

