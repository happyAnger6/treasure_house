#ifndef _EVENT_LOOP_H
#define _EVENT_LOOP_H 

#ifdef __cplusplus
extern "C"{
#endif

typedef void* event_loop_t;

extern event_loop_t event_loop_create();

extern int event_loop_run(event_loop_t ev_loop, long expired_ms);

#ifdef __cplusplus
}
#endif

#endif