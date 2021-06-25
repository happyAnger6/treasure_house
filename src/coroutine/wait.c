#include "wait.h"
#include "coroutine.h"

void init_wait_entry(wait_queue_t *wait, int flags)
{
    wait->flags = flags;
    wait->private = coroutine_current();
    wait->func = autoremove_wake_function;
    INIT_LIST_HEAD(&wait->task_list);
}

void prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state)
{
    co_spin_lock(&q->spin_lock);
    if (list_empty(&wait->task_list))
        _add_wait_queue(q, wait);

    coroutine_set_current_state(state);
    co_spin_unlock(&q->spin_lock);
}

void finish_wait(wait_queue_head_t *q, wait_queue_t *wait)
{
    co_spin_lock(&q->lock);
    if (!list_empty(&wait->task_list))
        list_del(&wait->task_list);
    co_spin_unlock(&q->lock);
}