/*
 * MxOS Kernel V0.1
 * Copyright (C) 2020 StephenZhou.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 1 tab == 4 spaces!
 */

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
