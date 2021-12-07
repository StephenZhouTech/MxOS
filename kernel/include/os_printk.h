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

#ifndef __MXOS_PRINTK_H__
#define __MXOS_PRINTK_H__

#include "os_configs.h"
#include <stdio.h>

#if OS_USE_PRINTK

    #if (OS_PRINTK_LEVEL <= OS_PRINTK_ERROR_LEVEL)
        #define OS_PRINTK_ERROR(format, args...) printf("[OS_ERROR]:"format"\r\n", ##args);
    #else
        #define OS_PRINTK_ERROR(format, args...)
    #endif

    #if (OS_PRINTK_LEVEL <= OS_PRINTK_WARNING_LEVEL)
        #define OS_PRINTK_WARNING(format, args...) printf("[OS_WARNING]:"format"\r\n", ##args);
    #else
        #define OS_PRINTK_WARNING(format, args...)
    #endif

    #if (OS_PRINTK_LEVEL <= OS_PRINTK_INFO_LEVEL)
        #define OS_PRINTK_INFO(format, args...) printf("[OS_INFO]:"format"\r\n", ##args);
    #else
        #define OS_PRINTK_INFO(format, args...)
    #endif

    #if (OS_PRINTK_LEVEL <= OS_PRINTK_DEBUG_LEVEL)
        #define OS_PRINTK_DEBUG(format, args...) printf("[OS_DEBUG]:"format"\r\n", ##args);
    #else
        #define OS_PRINTK_DEBUG(format, args...)
    #endif

#else

    #define OS_PRINTK_ERROR(format, args...)
    #define OS_PRINTK_WARNING(format, args...)
    #define OS_PRINTK_INFO(format, args...)
    #define OS_PRINTK_DEBUG(format, args...)

#endif // OS_USE_PRINTK

void OS_KernelStartPrint(void);

#endif // __MXOS_PRINTK_H__
