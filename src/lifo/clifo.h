
#ifndef __CLIFO_H
#define __CLIFO_H

#include <stddef.h>

typedef struct _clifo_s_ clifo_s;

clifo_s    *clifo_alloc(size_t size);
void        clifo_free(clifo_s *lifo);
size_t      clifo_size(clifo_s *lifo);
int         clifo_push(clifo_s *lifo, void *value);
void       *clifo_pop(clifo_s *lifo);

#endif /* __CSTACK_H */
