#ifndef __MXOS_TYPES_H__
#define __MXOS_TYPES_H__

#define OS_NULL                 0

#define OS_SIZE_BYTE            1
#define OS_SIZE_KB              (1024 * OS_SIZE_BYTE)
#define OS_SIZE_MB              (1024 * OS_SIZE_KB)

#define OS_FREQ_HZ              (1)
#define OS_FREQ_KHZ             (1000 * OS_FREQ_HZ)
#define OS_FREQ_MHZ             (1000 * OS_FREQ_KHZ)

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
