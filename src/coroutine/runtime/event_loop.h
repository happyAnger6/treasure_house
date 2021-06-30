#ifndef _EVENT_LOOP_H
#define _EVENT_LOOP_H 

#ifdef __cplusplus
extern "C"{
#endif

typedef void* event_loop_t;
typedef void* (event_callback)(void *args);

extern event_loop_t event_loop_create();

extern int event_loop_run(event_loop_t ev_loop, long expired_ms);

extern void event_loop_destory(event_loop_t ev_loop);

#ifdef __cplusplus
}
#endif

#endif