#ifndef _TIME_WHEEL_H
#define _TIME_WHEEL_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

typedef void* tw_handle_t;
typedef void* (*time_fn)(void *arg);
typedef int32_t time_id_t;

typedef enum {
    TW_SCHED_ONCE = 0,
    TW_SCHED_REPEAT,
    TW_SCHED_INVALID
} tw_sched_type_t;

typedef struct {
    time_fn fn;
    uint32_t expired_ms;
    tw_sched_type_t sched_type;
} tw_time_args_t;

extern tw_handle_t time_wheel_create(uint32_t accuracy_ms);
extern time_id_t time_wheel_add(tw_handle_t handle, tw_time_args_t args_t);
extern void time_wheel_del(tw_handle_t handle, time_id_t time_id);
extern void time_wheel_destory(tw_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif