#ifndef MICRO_PRINTF_H
#define MICRO_PRINTF_H

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

extern void printf_handler(char c);

#define printf __printf
u32 __printf(char *format, ...);

#define sprintf __sprintf
u32 __sprintf(char *buff, char *format, ...);

#define snprintf __snprintf
u32 __snprintf(char *buff, u32 count, char *format, ...);

#endif /* MICRO_PRINTF_H */
