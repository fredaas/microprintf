#include "printf.h"

#define FLAG_ZEROPAD_Msk  (1U << 0U)
#define FLAG_LEFTPAD_Msk  (1U << 1U)
#define FLAG_RIGHTPAD_Msk (1U << 2U)
#define FLAG_HEXUPPER_Msk (1U << 3U)
#define FLAG_SIGNED_Msk   (1U << 4U)
#define FLAG_OVERFLOW_Msk (1U << 5U)

#define PRINTF_MAX_BUFF_SIZE 256U

static u32 FLAGS = 0;
static s32 PAD_WIDTH = 0;
static u32 BYTE_COUNT = 0;
static u32 MAX_BYTE_COUNT = 0;

static char *ASCII_DIGIT_LOWER = "0123456789abcdef";
static char *ASCII_DIGIT_UPPER = "0123456789ABCDEF";

typedef void (*output_handler_t)(void *context, char c);

/**
 * @brief sprintf output handler
 */
static void fn_sprintf(void *context, char c)
{
    BYTE_COUNT++;
    char **p = (char **)context;
    *(*p)++ = c;
}

/**
 * @brief snprintf output handler
 */
static void fn_snprintf(void *context, char c)
{
    if (BYTE_COUNT == MAX_BYTE_COUNT)
    {
        FLAGS |= FLAG_OVERFLOW_Msk;
        return;
    }
    BYTE_COUNT++;
    char **p = (char **)context;
    *(*p)++ = c;
}

/**
 * @brief printf output handler
 */
static void fn_printf(void *context, char c)
{
    BYTE_COUNT++;
    printf_handler(c);
}

/**
 * @brief Pass a character stream to the output handler
 */
static void do_string(output_handler_t fn, void *context, char *buff)
{
    s32 padding = 0;

    if (PAD_WIDTH)
    {
        padding = PAD_WIDTH - strlen(buff);
        if (padding < 0)
            padding = 0;
    }

    /* Zero pad (0) */
    if (FLAGS & FLAG_ZEROPAD_Msk)
    {
        /* Print minus sign before zero padding */
        if (FLAGS & FLAG_SIGNED_Msk)
            fn(context, *buff++);

        for (u32 i = 0; i < padding; i++)
            fn(context, '0');
    }
    /* Left pad (-) */
    else if ((FLAGS & FLAG_LEFTPAD_Msk) || (!(FLAGS & FLAG_RIGHTPAD_Msk) && PAD_WIDTH))
    {
        for (u32 i = 0; i < padding; i++)
            fn(context, ' ');
    }

    while (*buff)
        fn(context, *buff++);

    /* Right pad (+) */
    if (FLAGS & FLAG_RIGHTPAD_Msk)
    {
        for (u32 i = 0; i < padding; i++)
            fn(context, ' ');
    }
}

/**
 * @brief Convert unsigned 64-bit integer to string
 */
static char *xutoa(char *buff, u64 x, u32 base)
{
    char *p = &buff[PRINTF_MAX_BUFF_SIZE];
    *--p = '\0';

    char *digit = ASCII_DIGIT_LOWER;

    if (FLAGS & FLAG_HEXUPPER_Msk)
        digit = ASCII_DIGIT_UPPER;

    /* Compare with buff + 1 to account for signed notation (see xitoa) */
    while (p > buff + 1)
    {
        *--p = digit[x % base];
        x = x / base;
        if (x == 0)
            break;
    }
    return p;
}

/**
 * @brief Convert signed 32-bit integer to string
 */
static char *xitoa(char *buff, s32 x, u32 base)
{
    if (x < 0)
    {
        char *p = xutoa(buff, -x, base);
        *--p = '-';
        FLAGS |= FLAG_SIGNED_Msk;
        return p;
    }
    return xutoa(buff, x, base);
}

/**
 * @brief Parse and decode string and call context handlers with appropriate arguments
 */
static u32 parse_string(output_handler_t fn, char *context, u32 count, char *format, va_list argp)
{
    BYTE_COUNT = 0;
    MAX_BYTE_COUNT = count;

    /* Storage for decoded representations of format strings (%s, %d, etc.) */
    char buff[PRINTF_MAX_BUFF_SIZE] = {0};

    /* Keep track of buffer pointer for sprintf, snprintf, etc. */
    void *context_ptr = (void *)&context;

    while (*format)
    {
        FLAGS = PAD_WIDTH = 0;

        if (*format == '%')
        {
            format++;

            switch (*format)
            {
            case '0':
                FLAGS |= FLAG_ZEROPAD_Msk;
                format++;
                break;
            case '-':
                FLAGS |= FLAG_LEFTPAD_Msk;
                format++;
                break;
            case '+':
                FLAGS |= FLAG_RIGHTPAD_Msk;
                format++;
                break;
            }

            if (*format >= '0' && *format <= '9')
                PAD_WIDTH = strtol(format, &format, 10);

            switch (*format)
            {
            case 'c':
                buff[0] = va_arg(argp, int);
                buff[1] = '\0';
                do_string(fn, context_ptr, buff);
                format++;
                break;
            case 'd':
                do_string(fn, context_ptr, xitoa(buff, va_arg(argp, s32), 10));
                format++;
                break;
            case 'u':
                do_string(fn, context_ptr, xutoa(buff, va_arg(argp, u64), 10));
                format++;
                break;
            case 's':
                do_string(fn, context_ptr, va_arg(argp, char *));
                format++;
                break;
            case 'b':
                do_string(fn, context_ptr, xutoa(buff, va_arg(argp, u32), 2));
                format++;
                break;
            case 'X':
                FLAGS |= FLAG_HEXUPPER_Msk;
            case 'x':
                do_string(fn, context_ptr, xutoa(buff, va_arg(argp, u32), 16));
                format++;
                break;
            case 'p':
                do_string(fn, context, "0x");
                do_string(fn, context_ptr, xutoa(buff, va_arg(argp, u64), 16));
                format++;
                break;
            }
        }
        else
        {
            fn(context_ptr, *format++);
        }

        if (FLAGS & FLAG_OVERFLOW_Msk)
            break;
    }

    /* MAX_BYTE_COUNT includes NULL byte, terminate if still space left */
    if (!(FLAGS & FLAG_OVERFLOW_Msk))
        fn(context_ptr, '\0');

    return FLAGS & FLAG_OVERFLOW_Msk ? BYTE_COUNT : BYTE_COUNT - 1;
}

u32 __printf(char *format, ...)
{
    va_list argp;
    va_start(argp, format);
    u32 num_bytes = parse_string(fn_printf, NULL, (u32)-1, format, argp);
    va_end(argp);
    return num_bytes;
}

u32 __sprintf(char *buff, char *format, ...)
{
    va_list argp;
    va_start(argp, format);
    u32 num_bytes = parse_string(fn_sprintf, buff, (u32)-1, format, argp);
    va_end(argp);
    return num_bytes;
}

u32 __snprintf(char *buff, u32 count, char *format, ...)
{
    va_list argp;
    va_start(argp, format);
    u32 num_bytes = parse_string(fn_snprintf, buff, count, format, argp);
    va_end(argp);
    return num_bytes;
}
