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

#ifndef __MXOS_TYPES_H__
#define __MXOS_TYPES_H__

#define OS_NULL                 0

#define OS_SIZE_BYTE            1
#define OS_SIZE_KB              (1024 * OS_SIZE_BYTE)
#define OS_SIZE_MB              (1024 * OS_SIZE_KB)

#define OS_FREQ_HZ              (1)
#define OS_FREQ_KHZ             (1000 * OS_FREQ_HZ)
#define OS_FREQ_MHZ             (1000 * OS_FREQ_KHZ)

#define OS_UINT32_MAX           (0xFFFFFFFFUL)
#define OS_UINT16_MAX           (0xFFFF)
#define OS_UINT8_MAX            (0xFF)

typedef   signed          char OS_Int8_t;
typedef   signed short     int OS_Int16_t;
typedef   signed           int OS_Int32_t;

typedef unsigned          char OS_Uint8_t;
typedef unsigned short     int OS_Uint16_t;
typedef unsigned           int OS_Uint32_t;

#define REG_32BIT_WR(addr, val) (* ((volatile OS_Uint32_t *)(addr)) ) = val
#define REG_32BIT_RD(addr)      (* ((volatile OS_Uint32_t *)(addr)) )

#define REG_16BIT_WR(addr, val) (* ((volatile OS_Uint16_t *)(addr)) ) = val
#define REG_16BIT_RD(addr)      (* ((volatile OS_Uint16_t *)(addr)) )

#define REG_8BIT_WR(addr, val)  (* ((volatile OS_Uint8_t *)(addr)) ) = val
#define REG_8BIT_RD(addr)       (* ((volatile OS_Uint8_t *)(addr)) )

#define OS_REG32(addr)          (* ((volatile OS_Uint32_t *)(addr)) )
#define OS_REG16(addr)          (* ((volatile OS_Uint16_t *)(addr)) )
#define OS_REG8(addr)           (* ((volatile OS_Uint8_t *)(addr)) )

#endif // __MXOS_TYPES_H__
