#ifndef _TH_ERROR_H
#define _TH_ERROR_H

enum ERROR_REASON{
    ERROR_SUCCESS = 0,
    ERROR_FAILED = 1,
    ERROR_NOMEM,
    ERROR_TOOBIG = 7,
    ERROR_MAX,
};
#endif