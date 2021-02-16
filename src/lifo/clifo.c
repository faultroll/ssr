
#include "clifo.h"
#include <stdint.h>
#include <stdlib.h>
#include "catomic.h"

typedef struct _clifo_node_s_ clifo_node_s;
typedef struct _clifo_head_s_ clifo_head_s;

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
