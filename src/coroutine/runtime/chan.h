#ifndef _COROUTINE_RUNTIME_CHAN_H
#define _COROUTINE_RUNTIME_CHAN_H

#ifdef __cplusplus
extern "C"{
#endif

#define CHAN_CLOSED 1

struct chan;
typedef struct chan* chan_t;

chan_t channel_open(uint32_t data_size);
int channel_write(chan_t chan, void *data);
int channel_read(chan_t chan, void *data);
int channel_close(chan_t chan);

#ifdef __cplusplus
}
#endif
