#ifndef _SYNC_H
#define _SYNC_H

typedef void* wait_group_t;

extern wait_group_t wait_group_create();
extern void wait_group_add(wait_group_t wg, int val);
extern void wait_group_done(wait_group_t wg);
extern void wait_group_wait(wait_group_t wg);

#endif