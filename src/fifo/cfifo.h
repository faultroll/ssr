
#ifndef __CFIFO_H
#define __CFIFO_H

#include <stddef.h>

typedef struct _cfifo_s_ cfifo_s;

cfifo_s    *cfifo_alloc(size_t size);
void        cfifo_free(cfifo_s *fifo);
size_t      cfifo_size(cfifo_s *fifo);
int         cfifo_push(cfifo_s *fifo, void *value);
void       *cfifo_pop(cfifo_s *fifo);

#endif /* __CSTACK_H */
