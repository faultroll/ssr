
#ifndef __CMPOOL_H
#define __CMPOOL_H

#include <stddef.h>

typedef struct _cmpool_s_ cmpool_s;

cmpool_s   *cmpool_init(size_t blknum, size_t blksz);
void        cmpool_fini(cmpool_s *mpool);
void       *cmpool_alloc(cmpool_s *mpool, size_t size);
void        cmpool_free(cmpool_s *mpool, void *ptr);

#endif /* __CMPOOL_H */
