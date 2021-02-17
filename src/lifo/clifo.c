
#include "clifo.h"
#include <stdint.h>
#include <stdlib.h>
#include "catomic.h"

// only CAS needed, not done yet
// https://github.com/javaf/elimination-backoff-stack
// https://github.com/mxinden/elimination-backoff-stack
// https://blog.csdn.net/weixin_45839894/article/details/105321818

typedef struct _clifo_node_s_ clifo_node_s;

struct _clifo_node_s_ {
    void *value;
};
struct _clifo_s_ {
    // int magic;
    // ATOMIC_VAR(bool) inited;
    ATOMIC_VAR(size_t) head;
    ARRAY_VAR(clifo_node_s) buffer;
};

// #define CLIFO_MAGIC ('l' << 24 | 'i' << 16 | 'f' << 8 | 'o' << 0)

clifo_s *clifo_alloc(size_t size)
{
    clifo_s *lifo = malloc(sizeof(clifo_s) + ARRAY_SIZE(clifo_node_s, size));
    if (NULL == lifo)
        return lifo;
    ARRAY_BOUND(&lifo->buffer) = size;

    // lifo->magic = CLIFO_MAGIC;
    ATOMIC_VAR_STOR(&lifo->head, 0);

    // ATOMIC_VAR_STOR(&lifo->inited, true);

    return lifo;
}

void clifo_free(clifo_s *lifo)
{
    free(lifo);
}

size_t clifo_size(clifo_s *lifo)
{
    return ATOMIC_VAR_LOAD(&lifo->head);
}

int clifo_push(clifo_s *lifo, void *value)
{
    // if (NULL == lifo || lifo->magic != CLIFO_MAGIC || !ATOMIC_VAR_LOAD(&lifo->inited))
    //     return;

    size_t size = ARRAY_BOUND(&lifo->buffer);
    size_t head = 0;
    do {
        head = ATOMIC_VAR_LOAD(&lifo->head);
        if (head == size - 1)
            // full
            return -1;
    } while (!ATOMIC_VAR_CAS(&lifo->head, &head, head + 1));
    // change value after head update, avoiding w/w race
    ARRAY_ARRAY(&lifo->buffer)[head].value = value;

    return 0;
}

void *clifo_pop(clifo_s *lifo)
{
    // if (NULL == lifo || lifo->magic != CLIFO_MAGIC || !ATOMIC_VAR_LOAD(&lifo->inited))
    //     return;

    void *value = NULL;
    size_t size = ARRAY_BOUND(&lifo->buffer);
    size_t head = 0;
    do {
        head = ATOMIC_VAR_LOAD(&lifo->head);
        if (head == 0)
            // empty
            return NULL;
        // change value before head update, avoiding r/w race
        value = ARRAY_ARRAY(&lifo->buffer)[head].value;
    } while (!ATOMIC_VAR_CAS(&lifo->head, &head, head - 1));

    return value;
}

#if 0 /* implement using list */

// from https://github.com/skeeto/lstack, DWCAS needed

struct _clifo_node_s_ {
    void *value;
    clifo_node_s *next;
};
struct _clifo_head_s_ {
    clifo_node_s *node;
    uintptr_t aba;
};
struct _clifo_s_ {
    clifo_node_s *node_buffer;
    ATOMIC_VAR(clifo_head_s) head, free;
    ATOMIC_VAR(size_t) size;
};

clifo_s *clifo_alloc(size_t size)
{
    clifo_s *lifo = malloc(sizeof(clifo_s));
    if (NULL == lifo)
        return lifo;

    clifo_head_s head_init = {NULL, 0};
    lifo->head = ATOMIC_VAR_INIT(head_init);
    lifo->size = ATOMIC_VAR_INIT(0);

    /* Pre-allocate all nodes. */
    lifo->node_buffer = malloc(size * sizeof(clifo_node_s));
    if (lifo->node_buffer == NULL) {
        free(lifo);
        lifo = NULL;
        return lifo;
    }

    for (size_t i = 0; i < size - 1; ++i)
        lifo->node_buffer[i].next = lifo->node_buffer + i + 1;
    lifo->node_buffer[size - 1].next = NULL;

    clifo_head_s free_init = {lifo->node_buffer, 0};
    lifo->free = ATOMIC_VAR_INIT(free_init);

    return lifo;
}

void clifo_free(clifo_s *lifo)
{
    free(lifo->node_buffer);
    free(lifo);
}

size_t clifo_size(clifo_s *lifo)
{
    return ATOMIC_VAR_LOAD(&lifo->size);
}

static clifo_node_s *__pop(ATOMIC_VAR(clifo_head_s) *head)
{
    clifo_head_s next, orig = ATOMIC_VAR_LOAD(head);
    do {
        if (orig.node == NULL)
            break;
        next.aba = orig.aba + 1;
        next.node = orig.node->next;
    } while (!ATOMIC_VAR_CAS(head, &orig, next));

    return orig.node;
}

static void __push(ATOMIC_VAR(clifo_head_s) *head, clifo_node_s *node)
{
    clifo_head_s next, orig = ATOMIC_VAR_LOAD(head);
    do {
        node->next = orig.node;
        next.aba = orig.aba + 1;
        next.node = node;
    } while (!ATOMIC_VAR_CAS(head, &orig, next));
}

int clifo_push(clifo_s *lifo, void *value)
{
    clifo_node_s *node = __pop(&lifo->free);
    if (node == NULL)
        return -1;
    node->value = value;
    __push(&lifo->head, node);
    ATOMIC_VAR_FAA(&lifo->size, 1);

    return 0;
}

void *clifo_pop(clifo_s *lifo)
{
    clifo_node_s *node = __pop(&lifo->head);
    if (node == NULL)
        return NULL;
    ATOMIC_VAR_FAA(&lifo->size, -1);
    void *value = node->value;
    __push(&lifo->free, node);

    return value;
}

#endif
