#include "mint.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// NOTE(Caleb): Hex dump format example:
// 0x0000  23 20 42 75 69 6c 64 20  2d 2d 2d 2d 2d 2d 2d 2d  |# Build --------|
#define __MINT_LOG_HEX_LINE_LEN 76

// Private Helper Declarations ---------------------------------------------------------------------

/// @brief Return the basename of a given path.
static char *__mint_basename(char *filename);

// Static Variables --------------------------------------------------------------------------------

#if MINT_LOG_BUFFER_SIZE > 0
static char S_LOG_MESSAGE_BUFFER[MINT_LOG_BUFFER_SIZE] = {0};
#endif

// Default Hook Implementations --------------------------------------------------------------------

__MINT_WEAK void mint_hook_write(const char *str, size_t size)
{
    printf("%.*s", (int)size, str);
}

__MINT_WEAK void mint_hook_on_assert_failed(void)
{
    while (1)
    {
        // Spin forever...
    }
}

// API Implementation ------------------------------------------------------------------------------

void __mint_log_impl_simple(char *file, int line, const char *format, ...)
{
    file = __mint_basename(file);

    const char *log_format_format = "%s:%d | %s\n";
    int         log_format_size   = snprintf(NULL, 0, log_format_format, file, line, format);
    if (log_format_size < 0)
    {
        return;
    }

    char log_format[log_format_size + 1];
    snprintf(log_format, log_format_size + 1, log_format_format, file, line, format);

    va_list args;

#if MINT_LOG_BUFFER_SIZE > 0
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

    mint_hook_write(log_message, log_message_size);
}

void __mint_log_hex_impl_simple(
    char *file, int line, const char *header, const void *data, size_t size)
{
    file = __mint_basename(file);

    const char *line_header_format = NULL;
    int         line_header_size   = 0;

    if (header)
    {
        line_header_format = "%s:%d | %s | ";
        line_header_size   = snprintf(NULL, 0, line_header_format, file, line, header);
    }
    else
    {
        line_header_format = "%s:%d | ";
        line_header_size   = snprintf(NULL, 0, line_header_format, file, line);
    }

    if (line_header_size < 0)
    {
        return;
    }

    char line_header[line_header_size + 1];
    if (header)
    {
        snprintf(line_header, line_header_size + 1, line_header_format, file, line, header);
    }
    else
    {
        snprintf(line_header, line_header_size + 1, line_header_format, file, line);
    }

    for (size_t i = 0; i < size; i += 16)
    {
#if MINT_LOG_BUFFER_SIZE > 0
        // NOTE(Caleb):
        // This assumes that the buffer is big enough to fit the line in all circumstances.
        char *line = S_LOG_MESSAGE_BUFFER;
#else
        char line[line_header_size + __MINT_LOG_HEX_LINE_LEN + 1];
#endif

        int line_size  = line_header_size + __MINT_LOG_HEX_LINE_LEN + 1;
        int render_idx = line_header_size;

        memcpy(line, line_header, line_header_size);

        // Line header + address ---------------------------

        render_idx +=
            snprintf(line + render_idx, line_size - render_idx, "0x%04X  ", (unsigned int)i);

        // Hex bytes ---------------------------------------

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                render_idx += snprintf(
                    line + render_idx,
                    line_size - render_idx,
                    "%02X ",
                    ((const uint8_t *)data)[i + j]);
            }
            else
            {
                render_idx += snprintf(line + render_idx, line_size - render_idx, "   ");
            }

            if (j == 7)
            {
                render_idx += snprintf(line + render_idx, line_size - render_idx, " ");
            }
        }

        // ASCII representation ----------------------------

        render_idx += snprintf(line + render_idx, line_size - render_idx, "|");

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                char c = ((const char *)data)[i + j];
                if (c < 32 || c > 126)
                {
                    c = '.';
                }

                render_idx += snprintf(line + render_idx, line_size - render_idx, "%c", c);
            }
        }

        render_idx += snprintf(line + render_idx, line_size - render_idx, "|\n");

        mint_hook_write(line, render_idx);
    }
}

// Private Helper Implementation -------------------------------------------------------------------

static char *__mint_basename(char *filename)
{
    char *basename = strrchr(filename, '/');
    return basename ? basename + 1 : filename;
}
