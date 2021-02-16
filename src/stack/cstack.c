
#include "cstack.h"
#include <stdint.h>
#include <stdlib.h>
#include "catomic.h"

typedef struct _cstack_node_s_ cstack_node_s;
typedef struct _cstack_head_s_ cstack_head_s;

struct _cstack_node_s_ {
    void *value;
    cstack_node_s *next;
};
struct _cstack_head_s_ {
    cstack_node_s *node;
    uintptr_t aba;
};
struct _cstack_s_ {
    cstack_node_s *node_buffer;
    ATOMIC_VAR(cstack_head_s) head, free;
    ATOMIC_VAR(size_t) size;
};

cstack_s *cstack_alloc(size_t size)
{
    cstack_s *stack = malloc(sizeof(cstack_s));
    if (NULL == stack)
        return stack;

    cstack_head_s head_init = {NULL, 0};
    stack->head = ATOMIC_VAR_INIT(head_init);
    stack->size = ATOMIC_VAR_INIT(0);

    /* Pre-allocate all nodes. */
    stack->node_buffer = malloc(size * sizeof(cstack_node_s));
    if (stack->node_buffer == NULL) {
        free(stack);
        stack = NULL;
        return stack;
    }

    for (size_t i = 0; i < size - 1; ++i)
        stack->node_buffer[i].next = stack->node_buffer + i + 1;
    stack->node_buffer[size - 1].next = NULL;

    cstack_head_s free_init = {stack->node_buffer, 0};
    stack->free = ATOMIC_VAR_INIT(free_init);

    return stack;
}

void cstack_free(cstack_s *stack)
{
    free(stack->node_buffer);
    free(stack);
}

size_t cstack_size(cstack_s *stack)
{
    return ATOMIC_VAR_LOAD(&stack->size);
}

static cstack_node_s *__pop(ATOMIC_VAR(cstack_head_s) *head)
{
    cstack_head_s next, orig = ATOMIC_VAR_LOAD(head);
    do {
        if (orig.node == NULL)
            break;
        next.aba = orig.aba + 1;
        next.node = orig.node->next;
    } while (!ATOMIC_VAR_CAS(head, &orig, next));

    return orig.node;
}

static void __push(ATOMIC_VAR(cstack_head_s) *head, cstack_node_s *node)
{
    cstack_head_s next, orig = ATOMIC_VAR_LOAD(head);
    do {
        node->next = orig.node;
        next.aba = orig.aba + 1;
        next.node = node;
    } while (!ATOMIC_VAR_CAS(head, &orig, next));
}

int cstack_push(cstack_s *stack, void *value)
{
    cstack_node_s *node = __pop(&stack->free);
    if (node == NULL)
        return -1;
    node->value = value;
    __push(&stack->head, node);
    ATOMIC_VAR_FAA(&stack->size, 1);

    return 0;
}

void *cstack_pop(cstack_s *stack)
{
    cstack_node_s *node = __pop(&stack->head);
    if (node == NULL)
        return NULL;
    ATOMIC_VAR_FAA(&stack->size, -1);
    void *value = node->value;
    __push(&stack->free, node);

    return value;
}
