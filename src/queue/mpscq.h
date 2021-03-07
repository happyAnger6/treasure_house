#ifndef _TH_MPSCQ_H
#define _TH_MPSCQ_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>

/* Multiple-producer single-consumer lock free queue, based upon the
* implementation from Dmitry Vyukov here:
* http://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue
*/

/* Advantages:
+ Intrusive. No need for additional internal nodes.
+ Wait-free and fast producers. One XCHG is maximum what one can get with multi-producer non-distributed queue.
+ Extremely fast consumer. On fast-path it's atomic-free, XCHG executed per node batch, in order to grab 'last item'.
+ No need for node order reversion. So pop operation is always O(1).
+ ABA-free.
+ No need for PDR. That is, one can use this algorithm out-of-the-box. 
   No need for thread registration/deregistration, periodic activity, deferred garbage etc.

Disadvantages:
- Push function is blocking wrt consumer. I.e. if producer blocked in (*), then consumer is blocked too.
   Fortunately 'window of inconsistency' is extremely small - producer must be blocked exactly in (*). 
  Actually it's disadvantage only as compared with totally lockfree algorithm. It's still much better lockbased algorithm.
*/

/* List node (include this in a data structure at the top, and add application
* fields after it - to simulate inheritance)
*/

struct mpscq_node
{
    struct mpscq_node* volatile  next;
};

struct mpscq
{
    union {
        char padding[TH_CACHELINE_SIZE];
        struct mpscq_node* volatile  head;
    };
    struct mpscq_node *tail;
    struct mpscq_node stub;
};

#define MPSCQ_STATIC_INIT(self) {&self.stub, &self.stub, {NULL}}

void mpscq_create(struct mpscq *self);

void mpscq_push(struct mpscq *self, struct mpscq_node *n);

struct mpscq_node *mpscq_pop(struct mpscq *self);

#ifdef __cplusplus
}
#endif

#endif
