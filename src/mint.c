#include "mint.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// NOTE(Caleb): Hex dump format example:
// 0x0000  23 20 42 75 69 6c 64 20  2d 2d 2d 2d 2d 2d 2d 2d  |# Build --------|
#define __MINT_LOG_HEX_LINE_LEN 76

#if MINT_ENABLE_COLORS
#define __MINT_COLOR_RESET   "\033[39m"
#define __MINT_COLOR_RED     "\033[91m"
#define __MINT_COLOR_GRAY    "\033[90m"
#define __MINT_COLOR_GREEN   "\033[92m"
#define __MINT_COLOR_MAGENTA "\033[95m"
#define __MINT_COLOR_YELLOW  "\033[93m"
#else
#define __MINT_COLOR_RESET   ""
#define __MINT_COLOR_RED     ""
#define __MINT_COLOR_GRAY    ""
#define __MINT_COLOR_GREEN   ""
#define __MINT_COLOR_MAGENTA ""
#define __MINT_COLOR_YELLOW  ""
#endif

// Private Helper Declarations ---------------------------------------------------------------------

/// @brief Return the basename of a given path.
static char *__mint_basename(char *filename);

static bool __mint_should_log(mint_level_e level, mint_id_t id);

// Static Variables --------------------------------------------------------------------------------

static char S_LOG_MESSAGE_BUFFER[MINT_LOG_BUFFER_SIZE] = {0};

#if MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
static const char *S_LEVEL_STRINGS[] = {
    [MINT_LEVEL_ALWAYS] = __MINT_COLOR_RESET "ALW",
    [MINT_LEVEL_NOTIFY] = __MINT_COLOR_MAGENTA "NTF",
    [MINT_LEVEL_FATAL]  = __MINT_COLOR_RED "FAT",
    [MINT_LEVEL_ERROR]  = __MINT_COLOR_RED "ERR",
    [MINT_LEVEL_WARN]   = __MINT_COLOR_YELLOW "WRN",
    [MINT_LEVEL_INFO]   = __MINT_COLOR_GREEN "INF",
    [MINT_LEVEL_DEBUG]  = __MINT_COLOR_GRAY "DBG",
    [MINT_LEVEL_TRACE]  = __MINT_COLOR_GRAY "TRC",
};
#endif

static mint_level_e s_global_level = MINT_LEVEL_DEBUG;

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

void mint_set_level(mint_id_t id, mint_level_e level)
{
#if MINT_API_LEVEL == MINT_API_LEVEL_BAREBONES
    (void)id;
    (void)level;
#elif MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
    (void)id;
    s_global_level = level;
#else // MINT_API_LEVEL == MINT_API_LEVEL_ADVANCED
    // TODO(Caleb): Add level and id filtering
    (void)id;
    (void)level;
#endif
}

void __mint_log_impl(
    mint_level_e level, mint_id_t id, char *file, int line, const char *format, ...)
{
    MINT_RETURN_IF(!__mint_should_log(level, id));

    file = __mint_basename(file);

#if MINT_API_LEVEL == MINT_API_LEVEL_BAREBONES
    (void)level;

    const char *log_format_format = "%s:%d | %s\n";
    int         log_format_size   = snprintf(NULL, 0, log_format_format, file, line, format);
    MINT_RETURN_IF(log_format_size < 0);

    char log_format[log_format_size + 1];
    (void)snprintf(log_format, log_format_size + 1, log_format_format, file, line, format);
#elif MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
    const char *log_format_format = "%s | %s:%d | %s\n%s";
    int         log_format_size   = snprintf(
        NULL, 0, log_format_format, S_LEVEL_STRINGS[level], file, line, format, __MINT_COLOR_RESET);
    MINT_RETURN_IF(log_format_size < 0);

    char log_format[log_format_size + 1];
    (void)snprintf(
        log_format,
        log_format_size + 1,
        log_format_format,
        S_LEVEL_STRINGS[level],
        file,
        line,
        format,
        __MINT_COLOR_RESET);
#else // MINT_API_LEVEL == MINT_API_LEVEL_ADVANCED
// ...
#endif

    va_list args;
    va_start(args, format);

    int log_message_size = vsnprintf(S_LOG_MESSAGE_BUFFER, MINT_LOG_BUFFER_SIZE, log_format, args);

    va_end(args);

    MINT_RETURN_IF(log_message_size < 0);
    mint_hook_write(S_LOG_MESSAGE_BUFFER, log_message_size);
}

void __mint_log_hex_impl(
    mint_level_e level,
    mint_id_t    id,
    char        *file,
    int          line,
    const char  *header,
    const void  *data,
    size_t       size)
{
    MINT_RETURN_IF(!__mint_should_log(level, id));

    file = __mint_basename(file);

    const char *line_header_format = NULL;
    int         line_header_size   = 0;

#if MINT_API_LEVEL == MINT_API_LEVEL_BAREBONES
    (void)level;

    line_header_format = (header) ? "%s:%d | %s | " : "%s:%d | ";
    if (header)
    {
        line_header_size = snprintf(NULL, 0, line_header_format, file, line, header);
    }
    else
    {
        line_header_size = snprintf(NULL, 0, line_header_format, file, line);
    }
#elif MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
    line_header_format = (header) ? "%s | %s:%d | %s | " : "%s | %s:%d | ";
    if (header)
    {
        line_header_size =
            snprintf(NULL, 0, line_header_format, S_LEVEL_STRINGS[level], file, line, header);
    }
    else
    {
        line_header_size =
            snprintf(NULL, 0, line_header_format, S_LEVEL_STRINGS[level], file, line);
    }
#else // MINT_API_LEVEL == MINT_API_LEVEL_ADVANCED
    // ...
#endif

    MINT_RETURN_IF(line_header_size < 0);
    char line_header[line_header_size + 1];

#if MINT_API_LEVEL == MINT_API_LEVEL_BAREBONES
    if (header)
    {
        snprintf(line_header, line_header_size + 1, line_header_format, file, line, header);
    }
    else
    {
        snprintf(line_header, line_header_size + 1, line_header_format, file, line);
    }
#elif MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
    if (header)
    {
        snprintf(
            line_header,
            line_header_size + 1,
            line_header_format,
            S_LEVEL_STRINGS[level],
            file,
            line,
            header);
    }
    else
    {
        snprintf(
            line_header,
            line_header_size + 1,
            line_header_format,
            S_LEVEL_STRINGS[level],
            file,
            line);
    }
#else // MINT_API_LEVEL == MINT_API_LEVEL_ADVANCED
    // ...
#endif

    for (size_t i = 0; i < size; i += 16)
    {
        int line_size  = line_header_size + __MINT_LOG_HEX_LINE_LEN + 1;
        int render_idx = line_header_size;

        memcpy(S_LOG_MESSAGE_BUFFER, line_header, line_header_size);

        // Line header + address ---------------------------

        render_idx += snprintf(
            S_LOG_MESSAGE_BUFFER + render_idx, line_size - render_idx, "0x%04X  ", (unsigned int)i);

        // Hex bytes ---------------------------------------

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx,
                    line_size - render_idx,
                    "%02X ",
                    ((const uint8_t *)data)[i + j]);
            }
            else
            {
                render_idx +=
                    snprintf(S_LOG_MESSAGE_BUFFER + render_idx, line_size - render_idx, "   ");
            }

            if (j == 7)
            {
                render_idx +=
                    snprintf(S_LOG_MESSAGE_BUFFER + render_idx, line_size - render_idx, " ");
            }
        }

        // ASCII representation ----------------------------

        render_idx += snprintf(S_LOG_MESSAGE_BUFFER + render_idx, line_size - render_idx, "|");

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                char c = ((const char *)data)[i + j];
                if (c < 32 || c > 126)
                {
                    c = '.';
                }

                render_idx +=
                    snprintf(S_LOG_MESSAGE_BUFFER + render_idx, line_size - render_idx, "%c", c);
            }
        }

        render_idx += snprintf(
            S_LOG_MESSAGE_BUFFER + render_idx, line_size - render_idx, "|\n%s", __MINT_COLOR_RESET);

        mint_hook_write(S_LOG_MESSAGE_BUFFER, render_idx);
    }
}

// Private Helper Implementation -------------------------------------------------------------------

static char *__mint_basename(char *filename)
{
    char *basename = strrchr(filename, '/');
    return basename ? basename + 1 : filename;
}

static bool __mint_should_log(mint_level_e level, mint_id_t id)
{
#if MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
    MINT_RETURN_IF(level > s_global_level, false);
#endif

    return true;
}