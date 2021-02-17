
#include <stddef.h>

#define ARRAY_VAR(_type) struct { size_t bound; _type array[]; }
#define ARRAY_SIZE(_type, _bound) (sizeof(ARRAY_VAR(_type)) + (_bound) * sizeof(_type))
#define ARRAY_BOUND(_ptr) ((_ptr)->bound)
#define ARRAY_ARRAY(_ptr) ((_ptr)->array)
// useless
// #define ARRAY_ALLOC(_type, _bound, _alloc) ({ ARRAY_VAR(_type) *__ptr = _alloc(ARRAY_SIZE(_type, _bound)); ARRAY_BOUND(__ptr) = _bound; __ptr; })
// #define ARRAY_FREE(_ptr, _free) (void)({ _free(_ptr); })

// carray.h
// typedef struct _carr_s_ carr_s;
// carr_s *carr_alloc(size_t bound, size_t size);
// void    carr_free(carr_s *arr);
// size_t  carr_size(carr_s *arr);
// void   *carr_data(carr_s *arr);
// carray.c
// struct _carr_s_ {
//     size_t bound;
//     /* void */char array[];
// };
// carr_s *carr_alloc(size_t bound, size_t size)
// {
//     carr_s *arr = malloc(sizeof(carr_s) + bound * size);
//     arr->bound = bound;

//     return arr;
// }
// void carr_free(carr_s *arr)
// {
//     free(arr);
// }
// size_t carr_size(carr_s *arr)
// {
//     return (arr->bound);
// }
// void *carr_data(carr_s *arr)
// {
//     return (void *)(arr->array);
// }
