#include "processor.h"
#include "coroutine_sched.h"

#include "asyncio.h"

void asyncio_sleep(long expired_ms)
{
    sched_t *sched = processor_get_sched();
    sched_delay(sched, delay_ms);
}