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

#ifndef __MXOS_ARCH_H__
#define __MXOS_ARCH_H__

#include "cpu.h"

#if ARCH_BYTE_ALIGNMENT == 32
    #define ARCH_BYTE_ALIGNMENT_MASK    ( 0x001f )
#endif

#if ARCH_BYTE_ALIGNMENT == 16
    #define ARCH_BYTE_ALIGNMENT_MASK    ( 0x000f )
#endif

#if ARCH_BYTE_ALIGNMENT == 8
    #define ARCH_BYTE_ALIGNMENT_MASK    ( 0x0007 )
#endif

#if ARCH_BYTE_ALIGNMENT == 4
    #define ARCH_BYTE_ALIGNMENT_MASK    ( 0x0003 )
#endif

#if ARCH_BYTE_ALIGNMENT == 2
    #define ARCH_BYTE_ALIGNMENT_MASK    ( 0x0001 )
#endif

#if ARCH_BYTE_ALIGNMENT == 1
    #define ARCH_BYTE_ALIGNMENT_MASK    ( 0x0000 )
#endif

#ifndef ARCH_BYTE_ALIGNMENT_MASK
    #error "Invalid ARCH_BYTE_ALIGNMENT definition"
#endif

#ifndef ARCH_NAME
    #define ARCH_NAME NULL
#endif

#endif // !__MXOS_ARCH_H__
