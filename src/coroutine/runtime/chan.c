#include "list.h"
#include "mutex.h"
#include "chan.h"
#include "wait.h"

typedef struct chan {
    char *data_buf;
    uint32_t data_cap;
    uint32_t elem_num;
    uint32_t elem_size;
    uint32_t send_idx;
    uint32_t recv_idx;
    uint32_t closed;
    wait_queue_head_t recv_waiters;
    wait_queue_head_t send_waiters;
    co_spin_lock lock;
} *chan_t;

chan_t channel_open(uint32_t elem_size)
{
    struct chan* c = malloc(sizeof(struct chan));
    c->data_buf = malloc(elem_size);
    c->data_cap = 1;
    c->elem_num = 0;
    c->elem_size = elem_size;
    c->send_idx = 0;
    c->recv_idx = 0;
    c->closed = 0;
    wait_queue_head_init(&c->recv_waiters);
    wait_queue_head_init(&c->send_waiters);
    co_spin_init(&c->lock);
}

int channel_write(chan_t c, void *data)
{
    if (c->closed)
        return -1;

again:
    wait_event(c->send_waiters, c->elem_num < c->data_cap);
    co_spin_lock(&c->lock);
   
    if (c->elem_num >= c->data_cap) {
        co_spin_unlock(&c->lock):
        goto again;
    }
    
    memcpy(c->data_buf + c->data_cnt * c->elem_size, data, c->elem_size);
    c->elem_num++;
    if (c->recv_waiters)
        wake_up(c->recv_waiters, WQ_FLAG_EXCLUSIVE);
  
    co_spin_unlock(&c->lock);
    return 0;
}

int channel_read(chan_t c, void *data)
{
    if (c->closed)
        return -1;

again:
    wait_event(c->recv_waiters, c->elem_num > 0);
    co_spin_lock(&c->lock);
   
    if (c->elem_num == 0 {
        co_spin_unlock(&c->lock):
        goto again;
    }
    
    memcpy(data, c->data_buf + (c->data_cnt - 1) * c->elem_size, c->elem_size);
    c->elem_num--;
    if (c->send_waiters)
        wake_up(c->send_waiters, WQ_FLAG_EXCLUSIVE);
  
    co_spin_unlock(&c->lock);
    return 0;
}