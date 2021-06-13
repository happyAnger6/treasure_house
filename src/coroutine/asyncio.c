#include "processor.h"
#include "coroutine_sched.h"

#include "asyncio.h"

void asyncio_sleep(int32_t expired_ms)
{
    sched_t *sched = processor_get_sched();
}