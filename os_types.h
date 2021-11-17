#ifndef __MXOS_TYPES_H__
#define __MXOS_TYPES_H__

#define OS_NULL                 0

#define OS_SIZE_BYTE            1
#define OS_SIZE_KB              (1024 * OS_SIZE_BYTE)
#define OS_SIZE_MB              (1024 * OS_SIZE_KB)

typedef   signed          char OS_Int8_t;
typedef   signed short     int OS_Int16_t;
typedef   signed           int OS_Int32_t;

typedef unsigned          char OS_Uint8_t;
typedef unsigned short     int OS_Uint16_t;
typedef unsigned           int OS_Uint32_t;

#endif // __MXOS_TYPES_H__
