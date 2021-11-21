#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define LOG_ERROR_LEVEL     3
#define LOG_WARNING_LEVEL   2
#define LOG_INFO_LEVEL      1
#define LOG_DEBUG_LEVEL     0

#define SYSTEM_LOG_LEVEL    LOG_DEBUG_LEVEL

#if (SYSTEM_LOG_LEVEL <= LOG_ERROR_LEVEL)
#define LOG_ERROR(format, args...) printf("[ERROR]:"format"\r\n", ##args);
#else
#define LOG_ERROR(format, args...)
#endif

#if (SYSTEM_LOG_LEVEL <= LOG_WARNING_LEVEL)
#define LOG_WARNING(format, args...) printf("[WARNING]:"format"\r\n", ##args);
#else
#define LOG_WARNING(format, args...)
#endif

#if (SYSTEM_LOG_LEVEL <= LOG_INFO_LEVEL)
#define LOG_INFO(format, args...) printf("[INFO]:"format"\r\n", ##args);
#else
#define LOG_INFO(format, args...)
#endif

#if (SYSTEM_LOG_LEVEL <= LOG_DEBUG_LEVEL)
#define LOG_DEBUG(format, args...) printf("[DEBUG]:"format"\r\n", ##args);
#else
#define LOG_DEBUG(format, args...)
#endif

void LogInit(void);

#endif // __LOG_H__
