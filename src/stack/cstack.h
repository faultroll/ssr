
#ifndef __CSTACK_H
#define __CSTACK_H

// from https://github.com/skeeto/lstack

#include <stddef.h>

typedef struct _cstack_s_ cstack_s;

cstack_s   *cstack_init(size_t size);
void        cstack_free(cstack_s *stack);
size_t      cstack_size(cstack_s *stack);
int         cstack_push(cstack_s *stack, void *value);
void       *cstack_pop(cstack_s *stack);

#endif /* __CSTACK_H */
