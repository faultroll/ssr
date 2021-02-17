
# super simple runtime

## user-space lock
    atomic --> spinlock
    lock-free means no mutex/rwlock or kernel-spin lock 
    lock-free is not a must, but concurrency/multi-thread safe is needed
## datastructure
    datastructure --> storage data
    fixed-size memory pool --> store int is enough
## no-seq
    map/set/vector (same key save once)
## seq
    fifo/lifo
    mpmc --> single/multi-target
## manager
    doesn't care where come from
### memory pool
    fixed-size mempool --> memblock(self explain in address and every block head, for multi memblock manager in multi-thread/process situation) --> array
    flex-size --> fixed-size + map
### thread pool
    user-space --> coroutine/protothread


# datastructure

datastructures are designed to store datas, there are two kinds of datastructures

reference
```
stl in c
stl(gnustl/...): https://zhuanlan.zhihu.com/p/23016264
https://blog.csdn.net/ssdlyf/article/details/46724881
https://ja.wikipedia.org/wiki/Libc
https://stackoverflow.com/questions/201413/c-analog-to-stl
https://stackoverflow.com/questions/2540/good-stl-like-library-for-c
https://stackoverflow.com/questions/668501/are-there-any-open-source-c-libraries-with-common-data-structures
https://stackoverflow.com/questions/890065/need-a-general-purpose-library-pure-c
apr(apache portable runtime)：http://apr.apache.org/
https://blog.csdn.net/xclshwd/article/details/103877102
https://stackoverflow.com/questions/296992/glib-v-apr-pros-and-cons-of-each
cstl：https://github.com/activesys/libcstl
https://github.com/P-p-H-d/mlib
ccan：https://github.com/rustyrussell/ccan/
sglib：https://github.com/stefanct/sglib
eastl: https://github.com/electronicarts/EASTL
other libs
data structure: https://www.cnblogs.com/skywang12345/p/3603935.html
tree: https://blog.csdn.net/qq_38499859/article/details/78857873
https://blog.csdn.net/u011240016/article/details/52823925
https://www.cnblogs.com/fanzhidongyzby/p/3475850.html
tree as array: https://webdocs.cs.ualberta.ca/~holte/T26/tree-as-array.html
https://stackoverflow.com/questions/2675756/efficient-array-storage-for-binary-tree/
https://stackoverflow.com/questions/6384658/if-i-store-a-binary-tree-in-an-array-how-do-i-avoid-the-wasted-space
circular array: https://stackoverflow.com/questions/42977769/how-to-create-a-circular-array
listlib: https://www.ee.ryerson.ca/~courses/ee8205/Data-Sheets/Tornado-VxWorks/vxworks/ref/lstLib.html
```
```
data structures in glibc(posix <search.h>)
https://stackoverflow.com/questions/14001652/does-standard-c-library-provides-linked-list-etc-data-structures
insque（list）: https://www.ee.ryerson.ca/~courses/ele709/susv4/functions/remque.html
https://www.jianshu.com/p/708ecbe5e71a
https://subho57.github.io/Noobs-Guide-to-C/headers/remque.html
binary tree（tsearch）: https://pubs.opengroup.org/onlinepubs/9699919799/functions/tsearch.html
https://zay.pub/2016/11/24/%E4%BD%BF%E7%94%A8tsearch%20twalk%E7%AD%89%E5%87%BD%E6%95%B0%E6%9E%84%E5%BB%BA%E3%80%81%E9%81%8D%E5%8E%86%E4%BA%8C%E5%8F%89%E6%A0%91/
https://www.gnu.org/software/libc/manual/html_node/Searching-and-Sorting.html
https://www.ibm.com/support/knowledgecenter/en/ssw_aix_71/com.ibm.aix.basetrf2/tsearch.htm
https://kazmax.zpp.jp/cmd/t/tsearch.3.html
```

- no-seq

    these datastructures are commonly used as database.
    they does not care about when data is stored. but we need key to retrieve them, so it is called key-value datastructure(like the cons/pair（数据(data)联合(construction)存储(storage)） in SICP)
    
    we usually have four operations as in SQL

    - （增）insert
    - （删）delete
    - （改）replace(update)
    - （查）find/iter(where)

    each operation have its own time and space complexity, thus we need algorithms (search/...) to get good performance (fast/small/...)

    reference
    ```
    hash/btree/skiplist/...（数据库本质数据结构）
    https://www.cnblogs.com/aspirant/p/11475295.html
    https://github.com/malbrain/database/
    https://www.cnblogs.com/vianzhang/p/7922426.html
    https://www.cnblogs.com/DilonWu/p/8857061.html
    https://stmatengss.github.io/blog/2018/11/04/Concepts-of-SkipList-Implementation-Concurrency-and-Lock-free/
    https://github.com/argv0/nbds
    https://github.com/greensky00/skiplist
    https://github.com/araina2/LockFree-HTM
    https://www.zhihu.com/question/52629893
    （Concurrent Skip List）https://github.com/facebook/folly
    https://www.cnblogs.com/fuzhe1989/p/3650303.html
    https://github.com/bhhbazinga/LockFreeHashTable
    https://github.com/LPD-EPFL/ASCYLIB
    https://github.com/concurrencykit/ck
    ```

- seq

    - fifo

        heap(queue)-->ring buffer-->bip buffer

        -  queue
        
            list（内存操作多）/array（不能无限增长，要数据搬移——
        
        - ring buffer

            full/empty（mirror memory不需要真分配内存，只需要index）/data连续性（mirror memory需要真正分配两块内存）

            ping-pong缓存本质是ringbuffer
    
    - lifo

        stack
    
    graph(two-dimension)/...
      

other old thinkings
```
？联合的方式，根据维度，可以是1、2、3、...维的，可以根据下标进行索引（array）
？因为是联合，所以scheme里的基本闭包（closure）结构就是cons
？离散数据间的关系（多维/数据分析） - iterator pattern
？参考sglib，可以分为两部分：iterator&memory，前者是数据间关系，后者是数据读取写入
？list和array这两个基础structure，对应两种模式，流（连续）&&包（索引、离散）
？FIFO/LIFO这种structure，说明联合有顺序需要，比如next/prev这种操作
```
reference
```
implementation and purpose:
https://stackoverflow.com/questions/1860999/list-of-fundamental-data-structures-what-am-i-missing
https://stackoverflow.com/questions/1514798/if-linked-list-and-array-are-fundamental-data-structures-what-type-of-data-struc
https://stackoverflow.com/questions/1539069/practical-uses-of-different-data-structures
https://stackoverflow.com/questions/4242634/class-vs-data-structure
https://stackoverflow.com/questions/3039513/type-safe-generic-data-structures-in-plain-old-c
https://stackoverflow.com/questions/4630377/explain-the-difference-between-a-data-structure-and-a-data-type
```

## performance

（迸发）concurrency, from spsc to mpmc(the producer/consumer or writer/reader module), or MT-safe/reentrant as a function.
concurrency需要考虑3个部分的同步(sync): w和w, r和r, w和r

- lock-free/CAS --> wait-free

主要解决w和w，r和r之间的同步（当然也需要考虑w和r之间的同步，但这部分不能完全解决）

``` c
// pattern: https://github.com/dryman/atomic_patterns
// lock
lock(v)
do A;
unlock(v)

// lock --> cas
if (try_lock(v))
  {if (equal(v, a))
    {swap(v, b);
    do A;}
  else
    {do B;}
  unlock(v);}
else
  {do B;}

// cas
if (cas(v, a, b))
  do A;
else
  do B;
```
```
wait-free
https://www.zhihu.com/question/295904223
https://blog.csdn.net/raomeng1/article/details/83145569
lock free
https://stidio.github.io/2017/01/cpp11_atomic_and_lockfree_program/
https://blog.csdn.net/weiwangchao_/article/details/51492823
https://blog.csdn.net/u013074465/article/details/47748619
https://github.com/boostcon/2011_presentations/tree/master/wed
https://alexpolt.github.io/atomic-data.html
Lock-free Stack：https://nullprogram.com/blog/2014/09/02/
lock-free MPSC(multi-producer-single-consumer) queue
ibm freelist(CAS): https://github.com/RossBencina/QueueWorld/blob/master/ALGORITHMS.txt
https://blog.csdn.net/lqt641/article/details/55058137
zerocopy：https://github.com/je-so/iqueue
每个writer一个queue，readers搜索所有queue（传送带模型？）：https://github.com/cameron314/concurrentqueue
https://moodycamel.com/blog
ABA problem：https://blog.csdn.net/yinkailin/article/details/79012532
https://www.stroustrup.com/isorc2010.pdf
https://lumian2015.github.io/lockFreeProgramming/aba-problem.html
https://community.intel.com/t5/Intel-C-Compiler/CAS-on-multi-core/td-p/876386
win32: https://docs.microsoft.com/en-us/windows/win32/dxtecharts/lockless-programming
cas&aba in database: https://blog.csdn.net/wufaliang003/article/details/78797203
Treiber stack suffer aba problem: https://stackoverflow.com/questions/46415027/c-treiber-stack-and-atomic-next-pointers
https://gpetri.github.io/MPRI-2017/index.html
cas and (ticket) spinlock: https://zhuanlan.zhihu.com/p/80727111
https://github.com/cyfdecyf/spinlock
https://ruby-china.org/topics/37916
https://www.cnblogs.com/shangdawei/p/3917117.html
https://winddoing.github.io/post/50889.html
https://blog.csdn.net/binling/article/details/50419103
https://www.zhihu.com/question/55764216
没有所谓的cas_u64，只有cas和dwcas（double word/wide），mcas/dcas是多个cas操作合为1个
需要用到的atomic操作主要为ptr(addr)和flag，如CAS/FAA/TAS，需要注意word和int/size_t/intptr_t等关系及int和intptr_t不一定相等 (The C standard does not know what a word is)
sizeof word: https://stackoverflow.com/questions/9988663/c-word-size-and-standard-size
https://stackoverflow.com/questions/35843365/how-to-detect-machine-word-size-in-c-c
Multiprocessors and Thread-Level Parallelism
https://ocw.nctu.edu.tw/course_detail-v.php?bgid=9&gid=0&nid=238
https://blog.csdn.net/yanghan1222/article/details/80275755
global lock（其实就是整个list内的lock而不是单个node struct内的lock）
https://stackoverflow.com/questions/51893772/does-rcu-synchronization-hold-only-a-global-lock-for-all-readers-of-different-da
https://stackoverflow.com/questions/229565/what-is-a-good-pattern-for-using-a-global-mutex-in-c
https://stackoverflow.com/questions/2332765/lock-mutex-semaphore-whats-the-difference
backoff（退避）: https://blog.csdn.net/tronyy/article/details/78898265
https://github.com/iiithf/concurrent-data-structures
```

- atomic

```
c11 stdatomic: http://ericnode.info/post/atomic_in_c11/
https://github.com/zenny-chen/C11-atomic-operations-in-detail
https://github.com/cdschreiber/c11
https://blog.codingnow.com/2021/01/skynet_stdatomic.html
atomic, -latomic：https://en.cppreference.com/w/c/atomic
https://stackoverflow.com/questions/30591313/why-does-g-still-require-latomic
c11 threads: https://stackoverflow.com/questions/14289634/thread-local-storage-class-specifier-in-c
```
volatile (跟atomic无关)
```
https://stackoverflow.com/questions/42182435/volatile-for-signal-handler-and-multi-threading
https://stackoverflow.com/questions/2484980/why-is-volatile-not-considered-useful-in-multithreaded-c-or-c-programming
https://www.airs.com/blog/archives/154
https://stackoverflow.com/questions/34521620/force-read-of-volatile-variable
https://stackoverflow.com/questions/8819095/concurrency-atomic-and-volatile-in-c11-memory-model
https://stackoverflow.com/questions/39358761/taking-a-semaphore-must-be-atomic-is-pintoss-sema-down-safe
```

- dealer/router?(the data has a destination in multi-consumer situation, however, we can use a pub/sub module, that all consumer can receive the data)

and there are some special use, eg. in some streaming buffer, consumers do not take when writer want to write(overlay), they use rcu mechanism (delete but free later)

```
lock-free本质是用户态的spinlock（cas），跟coroutine一样，最大的用处是高迸发的情况下，减少/防止用户态和内核态/上下文切换
```

- (non-blocking) memory reclamation

主要解决w和r之间的同步

```
mr有两种方案，一个是带gc的mempool，另一个是reader告知可以reclaim，即qsbr/epoch/hazardptr/...
https://stackoverflow.com/questions/40818465/explain-michael-scott-lock-free-queue-alorigthm
https://rusnikola.github.io
https://github.com/Pslydhh/PslyAlgorithm
https://github.com/mpoeter/xenium/
http://libcds.sourceforge.net/
hazard pointer: https://segmentfault.com/a/1190000012053016
http://ticki.github.io/blog/fearless-concurrency-with-hazard-pointers/
epoch: https://zhuanlan.zhihu.com/p/266025428
https://stackoverflow.com/questions/36573370/quiescent-state-based-reclamation-vs-epoch-based-reclamation
https://github.com/microsoft/L4
ref count: https://stidio.github.io/2017/01/cpp11_atomic_and_lockfree_program/
https://stackoverflow.com/questions/10074030/lock-free-reference-counting
https://stackoverflow.com/questions/37286702/lock-free-reference-counting-and-c-smart-pointers
可以将ref变量放在node内，此时rw_lock即ref++，rw_unlock即ref--
fast/slow path: https://blog.csdn.net/City_of_skey/article/details/84641308
https://stackoverflow.com/questions/45970525/is-the-example-in-the-membarrier-man-page-pointless-in-x86
https://stackoverflow.com/questions/52351397/is-there-a-penalty-when-baseoffset-is-in-a-different-page-than-the-base
rcu/tinyrcu
https://zhuanlan.zhihu.com/p/89439043
https://blog.csdn.net/wendowswd/article/details/90575606
https://www.cnblogs.com/bbqzsl/archive/2004/01/13/6842258.html
https://lwn.net/Articles/541037/
https://github.com/urcu/userspace-rcu
rcu本质是write-block的rwlock（read-block）
其模型上就是个pub/sub，write == publisher，reader == subscriber
每次writer执行synchronize_rcu就是发布变更（之前的操作叫做update），而发布变更要等之前进入rcu_read_lock的全部reader都读完，再执行后续语句（之后的操作叫reclaim）
如果需要不阻塞的write，就用call_rcu，把reclaim的操作放到单独地方（线程）做，如果是做free操作，其实就类似gc（user-driven）
rcu_assign_pointer/rcu_dereference完全没有必要（作用是做update操作内的同步）
hazard pointer: https://www.cnblogs.com/catch/p/5129586.html
epoch && rcu
http://www.yebangyu.org/blog/2016/09/09/epochbasedreclamation/
https://blog.csdn.net/solotzg/article/details/81050368
https://www.zhihu.com/question/406428296
https://stackoverflow.com/questions/36573370/quiescent-state-based-reclamation-vs-epoch-based-reclamation
https://aturon.github.io/blog/2015/08/27/epoch/
https://www.cnblogs.com/kekec/p/13795984.html
https://blog.csdn.net/zhangyifei216/article/details/52767236
http://www.read.seas.harvard.edu/cs161/2019/lectures/lecture18/
https://github.com/krakjoe/apcu/issues/345
https://gitee.com/mutouyun/lock-free-queue/tree/master
http://chonghw.github.io/
https://stackoverflow.com/questions/2599238/are-memory-barriers-necessary-for-atomic-reference-counting-shared-immutable-dat
https://github.com/rmind/libqsbr
https://github.com/mpoeter/emr
barrier: https://stackoverflow.com/questions/16975935/whats-the-purpose-of-compiler-barrier
gc: https://stackoverflow.com/questions/17646509/why-do-garbage-collectors-wait-before-deallocating
```

## target
- pure c(c99/c11/...)
- 库依赖越少越好（最好不依赖libc，比如malloc这些函数做成回调或者宏，这样还能兼顾multi-process的情况，比如malloc直接在sharemempool内申请内存）
- MT-safe && reentrant (concurrency)
- lock-free or user-space ticket spinlock（CAS/FAA等最好用宏表示，这样便于统一替换成一个atomic.h）
### content
- atomic(faa/cas/...)
- user-space lock(spin/rw/...)/thread(coroutine)
- task manager
  threadpool
- memory manager
  mempool 
  ```
  不关心内存哪里来的，只管理内存，如sharemempool仅仅是从sharemem分配大块内存
  https://github.com/userpro/MemoryPool
  https://github.com/dcshi/ncx_mempool
  fixed size mempool(无依赖) --> mempool(list等依赖)
  故lock-free库可以先开发fixed-size mempool，再开发table(hash/list/...)和fifo，其malloc可以用mempool管理，再用这两个开发flex-size mempool/threadpool/...
  ```

# manager

## threadpool

```
threadpool本质就是workqueue，一个mpmc的queue，多个thread取，哪个取到就哪个做这个job，thread越多就是取得越快
把threadpool和coroutine结合起来，看本质，就是user-space kernel，thread/core/cpu即worker，coroutine/thread即job，需要在job内手动yield，表示job中断，然后将job重新投回threadpool内（送到workqueue），下次resume（resume前需判断status，否则送入queue但未yield时resume会有问题）时便从中断处继续（需要保存context），哪个worker继续不影响job，这个流程不就是kernel的thread模型么？
```

