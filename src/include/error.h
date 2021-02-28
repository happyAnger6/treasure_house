#ifndef _TH_ERROR_H
#define _TH_ERROR_H

#ifdef __cplusplus
extern "C"{
#endif


enum ERROR_REASON{
    ERROR_SUCCESS = 0,
    ERROR_FAILED = 1,
    ERROR_NOMEM,
    ERROR_TOOBIG = 7,
    ERROR_MAX,
};

#ifdef __cplusplus
extern "C"}
#endif
#endif