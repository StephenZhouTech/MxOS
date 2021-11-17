#ifndef __MXOS_LIB_H__
#define __MXOS_LIB_H__

#include "os_types.h"

static inline void *OS_Memset(void *pbuf, OS_Uint8_t val, OS_Uint32_t count)
{
    OS_Uint8_t *_pbuf = (OS_Uint8_t *)pbuf;

    while (count--)
        *_pbuf++ = val;

    return pbuf;
}

static inline void *OS_Memcpy(void *dest, const void *src, OS_Uint32_t count)
{
    OS_Uint8_t *_dest = (OS_Uint8_t *)dest;
    const OS_Uint8_t *_src = (OS_Uint8_t *)src;

    while (count--)
        *_dest++ = *_src++;

    return dest;
}

static inline OS_Int32_t OS_Memcmp(const void *cs, const void *ct, OS_Uint32_t count)
{
    const OS_Uint8_t *su1, *su2;
    OS_Int32_t res = 0;

    for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
        if ((res = *su1 - *su2) != 0)
            break;
    return res;
}

static inline OS_Int32_t OS_DataAlign(OS_Uint32_t data, OS_Uint32_t align, OS_Uint32_t align_mask)
{
    if (data & align_mask)
    {
        data &= (~align_mask);
        data += align;
    }
    return data;
}

#endif // __MXOS_LIB_H__
