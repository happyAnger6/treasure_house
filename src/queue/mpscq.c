#include "atm.h"
#include "mpscq.h"

void mpscq_create(struct mpscq* self)
{
    self->head = &self->stub;
    self->tail = &self->stub;
    self->stub.next = 0;
}

/*
* init: head = &stub;
* one push: head = new_node; prev=&stub; stub->next=new;
* two push: 
                one updat head  = new_node1 and prev = old_head;
                another one update head = new_node2 and prev = new_node1;
                old_head->next = new_node1; and new_node1->next = new_node2;
*/
void mpscq_push(struct mpscq* self, struct mpscq_node* n)
{
    n->next = 0;
    struct mpscq_node* prev = atm_full_xchg((atm_t)&self->head, (atm_t)n);
    prev->next = n;
}

struct mpscq_node *mpscq_pop(struct mpscq* self)
{
    struct mpscq_node* tail = self->tail;
    struct mpscq_node* next = tail->next;
    if (tail == &self->stub) /*pop first*/
    {
        if (NULL == next)
            return 0;
        self->tail = next;
        tail = next;
        next = next->next;
    }
    if (next) /*check if have two, if so we can imediately pop*/
    {
        self->tail = next;
        return tail;
    }

    struct mpscq_node* head = self->head;
    if (tail != head) /*at the same time, another push called*/
        return NULL;

    mpscq_push(self, &self->stub); /*no matter another push first or we first, call all right*/
    next = tail->next;
    if (next)
    {
        self->tail = next;
        return tail;
    }
    return NULL;
} 

