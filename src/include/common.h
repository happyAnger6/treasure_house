#ifndef _TH_COMMON_H
#define _TH_COMMON_H

#ifdef __cplusplus
extern "C"{
#endif


#define __TH_ALIGN(x, a) __TH_ALIGN_MASK(x, (typeof(x))(a) - 1)
#define __TH_ALIGN_MASK (x, mask) (((x) + (mask)) & ~(mask))

#ifdef __cplusplus
}
#endif

#endif
