
# super simple runtime

## user-space lock
    atomic --> spinlock
    lock-free means no mutex/rwlock or kernel-spin lock 
    link -->
    lock-free is not a must, but concurrency/multi-thread safe is needed
## datastructure
    datastructure --> storage data
    fixed-size memory pool --> store int is enough
### no-seq
    map/set/vector (same key save once)
### seq
    fifo/lifo
    mpmc --> single/multi-target
## manager
    doesn't care where come from
### memory pool
    flex-size --> fixed-size + map
### thread pool
    user-space --> coroutine/protothread
