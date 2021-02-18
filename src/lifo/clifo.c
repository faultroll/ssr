
#include "clifo.h"
#include <stdint.h>
#include <stdlib.h>
#include "catomic.h"
#include "carray.h"

// only CAS needed

// node state
enum {
    NST_EMPTY,
    NST_BUSY, // mutex, preempt
    NST_FULL,
};
// lock type
enum {
    NST_PUSH,
    NST_POP,
};
// static ATOMIC_VAR(int) g_pplock_cnt = 0;
#define NST_PP_LOCK(_lock, _type) do { const int __old = (NST_PUSH == _type) ? NST_EMPTY : NST_FULL; while (!ATOMIC_VAR_CAS(&(_lock), &__old, NST_BUSY)) {} /* printf("(%d)(%d) lock (%d), cur (%d)\n", _type, ATOMIC_VAR_FAA(&g_pplock_cnt, 1), head, ATOMIC_VAR_LOAD(&_lock)); */ } while (0)
#define NST_PP_UNLOCK(_lock, _type) do { const int __new = (NST_PUSH == _type) ? NST_FULL : NST_EMPTY; ATOMIC_VAR_STOR(&(_lock), __new); /* const int __old = NST_BUSY; while (!ATOMIC_VAR_CAS(&(_lock), &__old, __new)) { printf("(%d)(%d) unlock (%d) error, cur (%d)\n", _type, ATOMIC_VAR_FAA(&g_pplock_cnt, 1), head, ATOMIC_VAR_LOAD(&_lock)); } */ } while (0)
// api
#define NST_LOCK ATOMIC_VAR(int)
#define NST_LOCK_INIT(_lock) ATOMIC_VAR_STOR(&(_lock), NST_EMPTY)
#define NST_PUSH_LOCK(_lock) NST_PP_LOCK((_lock), NST_PUSH)
#define NST_PUSH_UNLOCK(_lock) NST_PP_UNLOCK((_lock), NST_PUSH)
#define NST_POP_LOCK(_lock) NST_PP_LOCK((_lock), NST_POP)
#define NST_POP_UNLOCK(_lock) NST_PP_UNLOCK((_lock), NST_POP)

typedef struct _clifo_node_s_ clifo_node_s;

struct _clifo_node_s_ {
    void *value;
    NST_LOCK lock;
};
struct _clifo_s_ {
    ATOMIC_VAR(size_t) head;
    ARRAY_VAR(clifo_node_s) buffer;
};

clifo_s *clifo_alloc(size_t size)
{
    clifo_s *lifo = malloc(sizeof(clifo_s) + ARRAY_SIZE(clifo_node_s, size));
    if (NULL == lifo)
        return lifo;
    ARRAY_BOUND(&lifo->buffer) = size;

    ATOMIC_VAR_STOR(&lifo->head, 0);
    for (int i = 0; i < ARRAY_BOUND(&lifo->buffer); ++i)
        NST_LOCK_INIT(ARRAY_ARRAY(&lifo->buffer)[i].lock);

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
    // https://blog.csdn.net/fulltopic/article/details/24480291
    // dual-data structure
    size_t size = ARRAY_BOUND(&lifo->buffer);
    size_t head = 0;
    do {
        head = ATOMIC_VAR_LOAD(&lifo->head);
        if (head == size - 1) {
            // full
            return -1;
        }
    } while (!ATOMIC_VAR_CAS(&lifo->head, &head, head + 1));

    // printf("[%s] (%d): (%d)\n", __func__, head, (int)(intptr_t)value);
    NST_PUSH_LOCK(ARRAY_ARRAY(&lifo->buffer)[head].lock);
    ARRAY_ARRAY(&lifo->buffer)[head].value = value;
    NST_PUSH_UNLOCK(ARRAY_ARRAY(&lifo->buffer)[head].lock);

    return 0;
}

void *clifo_pop(clifo_s *lifo)
{
    // dual-data structure
    void *value = NULL;
    size_t size = ARRAY_BOUND(&lifo->buffer);
    size_t head = 0;
    do {
        head = ATOMIC_VAR_LOAD(&lifo->head);
        if (head == 0) {
            // empty
            return NULL;
        }
    } while (!ATOMIC_VAR_CAS(&lifo->head, &head, head - 1));

    NST_POP_LOCK(ARRAY_ARRAY(&lifo->buffer)[head - 1].lock);
    value = ARRAY_ARRAY(&lifo->buffer)[head - 1].value;
    NST_POP_UNLOCK(ARRAY_ARRAY(&lifo->buffer)[head - 1].lock);
    // printf("[%s] (%d): (%d)\n", __func__, head - 1, (int)(intptr_t)value);

    return value;
}

/*

// https://github.com/javaf/elimination-backoff-stack
// https://blog.csdn.net/weixin_45839894/article/details/105321818
// https://cs.nyu.edu/wies/teaching/ppc-14/material/lecture07.pdf

{
    // 1. Create a new node with given value.
    // 2. Try pushing it to stack.
    // 3a. If successful, return.
    // 3b. Otherwise, try exchanging on elimination array.
    // 4a. If found a matching pop, return.
    // 4b. Otherwise, retry 2.
    Node<T> n = new Node<>(x); // 1
    while (true) {
        if (tryPush(n)) // 2
            return;  // 3a
        try {
            T y = eliminationArray.visit(x); // 3b
            if (y == null) // 4a
                return;
        } catch (TimeoutException e) {
            // 4b
        }
    }
}

{
    // 1. Try popping a node from stack.
    // 2a. If successful, return node's value
    // 2b. Otherwise, try exchanging on elimination array.
    // 3a. If found a matching push, return its value.
    // 3b. Otherwise, retry 1.
    while (true) {
        Node<T> n = tryPop(); // 1
        if (n != null) // 2a
            return n.value;
        try {
            T y = eliminationArray.visit(null); // 2b
            if (y != null) // 3a
                return y;
        } catch (TimeoutException e) { // 3b

        }
    }
}

protected boolean tryPush(Node<T> n)
{
    // 1. Get stack top.
    // 2. Set node's next to top.
    // 3. Try push node at top (CAS).
    Node<T> m = top.get(); // 1
    n.next = m; // 2
    return top.compareAndSet(m, n); // 3
}

protected Node<T> tryPop()
{
    // 1. Get stack top, and ensure stack not empty.
    // 2. Try pop node at top, and set top to next (CAS).
    Node<T> m = top.get(); // 1
    if (m == null)
        throw new EmptyStackException(); // 1
    Node<T> n = m.next; // 2
    return top.compareAndSet(m, n) ? m : null; // 2
}

// Elimination array provides a list of exchangers which
// are picked at random for a given value.
class EliminationArray<T>
{
    // exchangers: array of exchangers
    // TIMEOUT: exchange timeout number
    // UNIT: exchange timeout unit
    // random: random number generator
    Exchanger<T>[] exchangers;
    final long TIMEOUT;
    final TimeUnit UNIT;
    Random random;
};

public EliminationArray(int capacity, long timeout, TimeUnit unit)
{
    exchangers = new Exchanger[capacity];
    for (int i = 0; i < capacity; i++)
        exchangers[i] = new Exchanger<>();
    random = new Random();
    TIMEOUT = timeout;
    UNIT = unit;
}

public T visit(T x)
{
    // 1. Try exchanging value on a random exchanger.
    int i = random.nextInt(exchangers.length);
    return exchangers[i].exchange(x, TIMEOUT, UNIT); // 1
}

// Exchanger is a lock-free object that permits two threads
// to exchange values, within a time limit.
class Exchanger<T>
{
    // slot: stores value and stamp
    // EMPTY: slot has no value.
    // WAITING: slot has 1st value, waiting for 2nd.
    // BUSY: slot has 2nd value, waiting to be empty.
    AtomicStampedReference<T> slot;
    static final int EMPTY = 0;
    static final int WAITING = 1;
    static final int BUSY = 2;
};

public T exchange(T y, long timeout, TimeUnit unit)
{
    // 1. Calculate last wait time.
    // 2. If wait time exceeded, then throw expection.
    // 3. Get slot value and stamp.
    // 4a. If slot is EMPTY (no value):
    // 4b. Try adding 1st value to slot, else retry 2.
    // 4c. Try getting 2nd value from slot, within time limit.
    // 5a. If slot is WAITING (has 1st value):
    // 5b. Try adding 2nd value to slot, else retry 2.
    // 5c. Return 1st value.
    // 6a. If slot is BUSY (has 2nd value):
    // 6b. Retry 2.
    long w = unit.toNanos(timeout); // 1
    long W = System.nanoTime() + w; // 1
    int[] stamp = {EMPTY};
    while (System.nanoTime() < W) { // 2
        T x = slot.get(stamp); // 3
        switch (stamp[0]) { // 3
            case EMPTY: // 4
                if (addA(y)) { // 4
                    while (System.nanoTime() < W) // 4
                        if ((x = removeB()) != null)
                            return x; // 4
                    throw new TimeoutException(); // 5
                }
                break;
            case WAITING: // 7
                if (addB(x, y)) // 7
                    return x; // 7
                break;
            case BUSY: // 8
                break; // 8
            default:
        }
    }
    throw new TimeoutException(); // 2
}

private boolean addA(T y)
{
    // 1. Add 1st value to slot.
    // 2. Set its stamp as WAITING (for 2nd).
    return slot.compareAndSet(null, y, EMPTY, WAITING); // 1, 2
}

private boolean addB(T x, T y)
{
    // 1. Add 2nd value to slot.
    // 2. Set its stamp as BUSY (for 1st to remove).
    return slot.compareAndSet(x, y, WAITING, BUSY); // 1, 2
}

private T removeB()
{
    // 1. If stamp is not BUSY (no 2nd value in slot), exit.
    // 2. Set slot as EMPTY, and get 2nd value from slot.
    int[] stamp = {EMPTY};
    T x = slot.get(stamp); // 1
    if (stamp[0] != BUSY)
        return null; // 1
    slot.set(null, EMPTY); // 2
    return x; // 2
} */

/* class Node
{
    Node tl;
    int val;
}
class NodePtr
{
    Node val;
} TOP;
class TidPtr
{
    int val;
} clash;

void push(int e)
{
    Node y, n;
    TID hisId;
    y = new ();
    y->val = e;
    while (true) {
        n = TOP->val;
        y->tl = n;
        if (cas(TOP->val, n, y))
            return;
        //elimination scheme
        TidPtr t = new TidPrt();
        t->val = e;
        if (cas(clash, null, t)) {
            wait(DELAY);
            //not eliminated
            if (cas(clash, t, null))
                continue;
            //eliminated
            else
                break;
        }
    }
}

int pop()
{
    Node y, z;
    int t;
    TID hisId;
    while (true) {
        y = TOP->val;
        if (y == 0)
            return EMPTY;
        z = y->tl;
        t = y->val;
        if (cas(TOP->val, y, z)
            return t;
            //elimination scheme
            pusher = clash;
        while (pusher != null) {
            if (cas(clash, pusher, null))
                    //eliminated push
                    return pusher->val;
            }
    }
} */

/* // from https://github.com/skeeto/lstack, DWCAS needed

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

    // Pre-allocate all nodes.
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
} */

