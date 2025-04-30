#include "mint.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char *__mint_basename(char *filename);
static void  __mint_write(const char *str, size_t size);

#if (MINT_LOG_BUFFER_SIZE > 0)
static char S_LOG_MESSAGE_BUFFER[MINT_LOG_BUFFER_SIZE] = {0};
#endif

__MINT_WEAK void mint_hook_on_assert_failed(void)
{
    while (1)
    {
        // Spin forever...
    }
}

void __mint_log_impl(char *file, int line, const char *format, ...)
{
    file = __mint_basename(file);

    const char *log_format_format = "%s:%d - %s\n";
    int         log_format_size   = snprintf(NULL, 0, log_format_format, file, line, format);
    if (log_format_size < 0)
    {
        return;
    }

    char log_format[log_format_size + 1];
    snprintf(log_format, log_format_size + 1, log_format_format, file, line, format);

    va_list args;

#if (MINT_LOG_BUFFER_SIZE > 0)
    int   log_message_size = MINT_LOG_BUFFER_SIZE;
    char *log_message      = S_LOG_MESSAGE_BUFFER;
#else
    va_start(args, format);
    int log_message_size = vsnprintf(NULL, 0, log_format, args);
    va_end(args);

    if (log_message_size < 0)
    {
        return;
    }

    log_message_size++; // NOTE(Caleb): Include NULL terminator
    char log_message[log_message_size];
#endif

    va_start(args, format);
    vsnprintf(log_message, log_message_size, log_format, args);
    va_end(args);

    __mint_write(log_message, log_message_size);
}

static char *__mint_basename(char *filename)
{
    char *basename = strrchr(filename, '/');
    return basename ? basename + 1 : filename;
}

static void __mint_write(const char *str, size_t size)
{
    printf("%.*s", (int)size, str);
}
