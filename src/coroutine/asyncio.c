#include "processor.h"
#include "coroutine_sched.h"

#include "asyncio.h"

void asyncio_sleep(long delay_ms)
{
    sched_t *sched = processors_get_sched();
    sched_delay(sched, delay_ms);
}