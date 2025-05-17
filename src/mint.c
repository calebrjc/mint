#include "mint.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define __MINT_MIN(__x, __y)    (((__x) < (__y)) ? (__x) : (__y))
#define __MINT_UNUSED(x)        (void)(x)

// NOTE(Caleb): Hex dump format example:
// 0x00000000  23 20 42 75 69 6c 64 20  2d 2d 2d 2d 2d 2d 2d 2d  |# Build --------|
#define __MINT_LOG_HEX_LINE_LEN 80

#if MINT_ENABLE_TIME
#define __MINT_TIME_FORMAT  "[%02d:%02d:%02d,%03d]"
#define __MINT_TIME_STR_LEN (sizeof("[00:00:00,000]") - 1)
#endif

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

/// @brief Return true if a log message should be output, and false otherwise.
/// @note For MINT_API_LEVEL_BAREBONES, this is always true.
/// @note For MINT_API_LEVEL_SIMPLE, this only takes the level into account.
static bool __mint_should_log(mint_id_t id, mint_level_e level);

#if MINT_ENABLE_TIME
/// @brief Format the system time as a string into the given buffer.
static void __mint_format_time(mint_time_t time, char *o_data, size_t size);
#endif

// Static Variables --------------------------------------------------------------------------------

static char S_LOG_MESSAGE_BUFFER[MINT_LOG_BUFFER_SIZE] = {0};

#if MINT_API_LEVEL >= MINT_API_LEVEL_SIMPLE
static const char *const S_LEVEL_STRINGS[] = {
    [MINT_LEVEL_ALWAYS] = "ALW",
    [MINT_LEVEL_NOTIFY] = "NTF",
    [MINT_LEVEL_FATAL]  = "FAT",
    [MINT_LEVEL_ERROR]  = "ERR",
    [MINT_LEVEL_WARN]   = "WRN",
    [MINT_LEVEL_INFO]   = "INF",
    [MINT_LEVEL_DEBUG]  = "DBG",
    [MINT_LEVEL_TRACE]  = "TRC",
};

static const char *const S_LEVEL_COLORS[] = {
    [MINT_LEVEL_ALWAYS] = __MINT_COLOR_RESET,
    [MINT_LEVEL_NOTIFY] = __MINT_COLOR_MAGENTA,
    [MINT_LEVEL_FATAL]  = __MINT_COLOR_RED,
    [MINT_LEVEL_ERROR]  = __MINT_COLOR_RED,
    [MINT_LEVEL_WARN]   = __MINT_COLOR_YELLOW,
    [MINT_LEVEL_INFO]   = __MINT_COLOR_GREEN,
    [MINT_LEVEL_DEBUG]  = __MINT_COLOR_GRAY,
    [MINT_LEVEL_TRACE]  = __MINT_COLOR_GRAY,
};
#endif

#if MINT_API_LEVEL > MINT_API_LEVEL_BAREBONES
static mint_level_e s_global_level = MINT_LEVEL_DEBUG;
#endif

#if MINT_API_LEVEL == MINT_API_LEVEL_ADVANCED
static mint_log_ctx_t s_log_contexts[MINT_LOG_ID_MAX] = {
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
    {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG}, {MINT_LEVEL_DEBUG, MINT_DEFAULT_TAG},
};
#endif

// Default Hook Implementations --------------------------------------------------------------------

__MINT_WEAK void mint_hook_write(const char *str, size_t size)
{
    printf("%.*s", (int)size, str);
}

__MINT_WEAK void mint_hook_on_assert_failed(void)
{
    // NOTE(Caleb): This library is generally made for the benefit of embedded systems, so
    // the default exit handler is to spin forever since we don't have access to exit() or abort().

    while (1)
    {
        // Spin forever...
    }
}

__MINT_WEAK void mint_hook_lock(void)
{
    // No-op
}

__MINT_WEAK void mint_hook_unlock(void)
{
    // No-op
}

__MINT_WEAK uint32_t mint_hook_get_time(void)
{
    return 0;
}

// API Implementation ------------------------------------------------------------------------------

#if MINT_API_LEVEL == MINT_API_LEVEL_BAREBONES
void __mint_log_impl(
    mint_id_t id, mint_level_e level, char *file, int line, const char *format, ...)
{
    __MINT_UNUSED(id);
    __MINT_UNUSED(level);

    MINT_RETURN_VOID_IF(!__mint_should_log(id, level));

    file = __mint_basename(file);

#if MINT_ENABLE_TIME
    char time_str[__MINT_TIME_STR_LEN + 1] = {0};
    __mint_format_time(mint_hook_get_time(), time_str, __MINT_TIME_STR_LEN + 1);

    const char *log_format_format = "%s %s:%d | %s\n";
    int log_format_size = snprintf(NULL, 0, log_format_format, time_str, file, line, format);
    MINT_RETURN_VOID_IF(log_format_size < 0);

    char log_format[log_format_size + 1];
    (void)snprintf(
        log_format, log_format_size + 1, log_format_format, time_str, file, line, format);
#else
    const char *log_format_format = "%s:%d | %s\n";
    int         log_format_size   = snprintf(NULL, 0, log_format_format, file, line, format);
    MINT_RETURN_VOID_IF(log_format_size < 0);

    char log_format[log_format_size + 1];
    (void)snprintf(log_format, log_format_size + 1, log_format_format, file, line, format);
#endif

    mint_hook_lock();

    va_list args;
    va_start(args, format);

    int log_message_size = vsnprintf(S_LOG_MESSAGE_BUFFER, MINT_LOG_BUFFER_SIZE, log_format, args);

    va_end(args);

    if (log_message_size < 0)
    {
        mint_hook_unlock();
        return;
    }

    log_message_size = __MINT_MIN(log_message_size, MINT_LOG_BUFFER_SIZE - 1);
    mint_hook_write(S_LOG_MESSAGE_BUFFER, log_message_size);

    mint_hook_unlock();
}

void __mint_log_hex_impl(
    mint_id_t    id,
    mint_level_e level,
    char        *file,
    int          line,
    const char  *header,
    const void  *data,
    size_t       size)
{
    __MINT_UNUSED(id);
    __MINT_UNUSED(level);

    MINT_RETURN_VOID_IF(!__mint_should_log(id, level));

    file = __mint_basename(file);

    const char *line_header_format = NULL;
    int         line_header_size   = 0;

#if MINT_ENABLE_TIME
    char time_str[__MINT_TIME_STR_LEN + 1] = {0};
    __mint_format_time(mint_hook_get_time(), time_str, __MINT_TIME_STR_LEN + 1);

    line_header_format = (header) ? "%s %s:%d | %s | " : "%s %s:%d | ";
    if (header)
    {
        line_header_size = snprintf(NULL, 0, line_header_format, time_str, file, line, header);
    }
    else
    {
        line_header_size = snprintf(NULL, 0, line_header_format, time_str, file, line);
    }

    // TODO(Caleb): Reconsider how to implement this without VLAs to have support for MSVC and C23
    MINT_RETURN_VOID_IF(line_header_size < 0);
    char line_header[line_header_size + 1];

    if (header)
    {
        snprintf(
            line_header, line_header_size + 1, line_header_format, time_str, file, line, header);
    }
    else
    {
        snprintf(line_header, line_header_size + 1, line_header_format, time_str, file, line);
    }
#else
    line_header_format = (header) ? "%s:%d | %s | " : "%s:%d | ";
    if (header)
    {
        line_header_size = snprintf(NULL, 0, line_header_format, file, line, header);
    }
    else
    {
        line_header_size = snprintf(NULL, 0, line_header_format, file, line);
    }

    // TODO(Caleb): Reconsider how to implement this without VLAs to have support for MSVC and C23
    MINT_RETURN_VOID_IF(line_header_size < 0);
    char line_header[line_header_size + 1];

    if (header)
    {
        snprintf(line_header, line_header_size + 1, line_header_format, file, line, header);
    }
    else
    {
        snprintf(line_header, line_header_size + 1, line_header_format, file, line);
    }
#endif

    mint_hook_lock();

    for (size_t i = 0; i < size; i += 16)
    {
        int render_idx = line_header_size;

        memcpy(S_LOG_MESSAGE_BUFFER, line_header, line_header_size);

        // Line header + address ---------------------------

        render_idx += snprintf(
            S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "0x%08zX  ", i);

        // Hex bytes ---------------------------------------

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx,
                    MINT_LOG_BUFFER_SIZE - render_idx,
                    "%02X ",
                    ((const uint8_t *)data)[i + j]);
            }
            else
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "   ");
            }

            if (j == 7)
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, " ");
            }
        }

        // ASCII representation ----------------------------

        render_idx +=
            snprintf(S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "|");

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                char c = ((const char *)data)[i + j];
                if (c < 32 || c > 126)
                {
                    c = '.';
                }

                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "%c", c);
            }
        }

        render_idx += snprintf(
            S_LOG_MESSAGE_BUFFER + render_idx,
            MINT_LOG_BUFFER_SIZE - render_idx,
            "|\n%s",
            __MINT_COLOR_RESET);

        mint_hook_write(S_LOG_MESSAGE_BUFFER, __MINT_MIN(render_idx, MINT_LOG_BUFFER_SIZE));
    }

    mint_hook_unlock();
}

static bool __mint_should_log(mint_id_t id, mint_level_e level)
{
    __MINT_UNUSED(id);
    __MINT_UNUSED(level);

    return true;
}
#endif

#if MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
void mint_set_level(mint_id_t id, mint_level_e level)
{
    __MINT_UNUSED(id);

    s_global_level = level;
}

void __mint_log_impl(
    mint_id_t id, mint_level_e level, char *file, int line, const char *format, ...)
{
    __MINT_UNUSED(id);

    MINT_RETURN_VOID_IF(!__mint_should_log(id, level));

    file = __mint_basename(file);

#if MINT_ENABLE_TIME
    char time_str[__MINT_TIME_STR_LEN + 1];
    __mint_format_time(mint_hook_get_time(), time_str, __MINT_TIME_STR_LEN + 1);

    const char *log_format_format = "%s%s %s | %s:%d | %s%s\n";
    int         log_format_size   = snprintf(
        NULL,
        0,
        log_format_format,
        S_LEVEL_COLORS[level],
        time_str,
        S_LEVEL_STRINGS[level],
        file,
        line,
        format,
        __MINT_COLOR_RESET);
    MINT_RETURN_VOID_IF(log_format_size < 0);

    char log_format[log_format_size + 1];
    (void)snprintf(
        log_format,
        log_format_size + 1,
        log_format_format,
        S_LEVEL_COLORS[level],
        time_str,
        S_LEVEL_STRINGS[level],
        file,
        line,
        format,
        __MINT_COLOR_RESET);
#else
    const char *log_format_format = "%s%s | %s:%d | %s%s\n";
    int         log_format_size   = snprintf(
        NULL,
        0,
        log_format_format,
        S_LEVEL_COLORS[level],
        S_LEVEL_STRINGS[level],
        file,
        line,
        format,
        __MINT_COLOR_RESET);
    MINT_RETURN_VOID_IF(log_format_size < 0);

    char log_format[log_format_size + 1];
    (void)snprintf(
        log_format,
        log_format_size + 1,
        log_format_format,
        S_LEVEL_COLORS[level],
        S_LEVEL_STRINGS[level],
        file,
        line,
        format,
        __MINT_COLOR_RESET);
#endif

    mint_hook_lock();

    va_list args;
    va_start(args, format);

    int log_message_size = vsnprintf(S_LOG_MESSAGE_BUFFER, MINT_LOG_BUFFER_SIZE, log_format, args);

    va_end(args);

    if (log_message_size < 0)
    {
        mint_hook_unlock();
        return;
    }

    log_message_size = __MINT_MIN(log_message_size, MINT_LOG_BUFFER_SIZE - 1);
    mint_hook_write(S_LOG_MESSAGE_BUFFER, log_message_size);

    mint_hook_unlock();
}

void __mint_log_hex_impl(
    mint_id_t    id,
    mint_level_e level,
    char        *file,
    int          line,
    const char  *header,
    const void  *data,
    size_t       size)
{
    __MINT_UNUSED(id);

    MINT_RETURN_VOID_IF(!__mint_should_log(id, level));

    file = __mint_basename(file);

    const char *line_header_format = NULL;
    int         line_header_size   = 0;

#if MINT_ENABLE_TIME
    char time_str[__MINT_TIME_STR_LEN + 1];
    __mint_format_time(mint_hook_get_time(), time_str, __MINT_TIME_STR_LEN + 1);

    line_header_format = (header) ? "%s%s %s | %s:%d | %s | " : "%s %s | %s:%d | ";
    if (header)
    {
        line_header_size = snprintf(
            NULL,
            0,
            line_header_format,
            S_LEVEL_COLORS[level],
            time_str,
            S_LEVEL_STRINGS[level],
            file,
            line,
            header);
    }
    else
    {
        line_header_size = snprintf(
            NULL,
            0,
            line_header_format,
            S_LEVEL_COLORS[level],
            time_str,
            S_LEVEL_STRINGS[level],
            file,
            line);
    }

    // TODO(Caleb): Reconsider how to implement this without VLAs to have support for MSVC and C23
    MINT_RETURN_VOID_IF(line_header_size < 0);
    char line_header[line_header_size + 1];

    if (header)
    {
        snprintf(
            line_header,
            line_header_size + 1,
            line_header_format,
            S_LEVEL_COLORS[level],
            time_str,
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
            S_LEVEL_COLORS[level],
            time_str,
            S_LEVEL_STRINGS[level],
            file,
            line);
    }
#else
    line_header_format = (header) ? "%s%s | %s:%d | %s | " : "%s | %s:%d | ";
    if (header)
    {
        line_header_size = snprintf(
            NULL,
            0,
            line_header_format,
            S_LEVEL_COLORS[level],
            S_LEVEL_STRINGS[level],
            file,
            line,
            header);
    }
    else
    {
        line_header_size = snprintf(
            NULL, 0, line_header_format, S_LEVEL_COLORS[level], S_LEVEL_STRINGS[level], file, line);
    }

    // TODO(Caleb): Reconsider how to implement this without VLAs to have support for MSVC and C23
    MINT_RETURN_VOID_IF(line_header_size < 0);
    char line_header[line_header_size + 1];

    if (header)
    {
        snprintf(
            line_header,
            line_header_size + 1,
            line_header_format,
            S_LEVEL_COLORS[level],
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
            S_LEVEL_COLORS[level],
            S_LEVEL_STRINGS[level],
            file,
            line);
    }
#endif

    mint_hook_lock();

    for (size_t i = 0; i < size; i += 16)
    {
        int render_idx = line_header_size;

        memcpy(S_LOG_MESSAGE_BUFFER, line_header, line_header_size);

        // Line header + address ---------------------------

        render_idx += snprintf(
            S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "0x%08zX  ", i);

        // Hex bytes ---------------------------------------

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx,
                    MINT_LOG_BUFFER_SIZE - render_idx,
                    "%02X ",
                    ((const uint8_t *)data)[i + j]);
            }
            else
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "   ");
            }

            if (j == 7)
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, " ");
            }
        }

        // ASCII representation ----------------------------

        render_idx +=
            snprintf(S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "|");

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                char c = ((const char *)data)[i + j];
                if (c < 32 || c > 126)
                {
                    c = '.';
                }

                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "%c", c);
            }
        }

        render_idx += snprintf(
            S_LOG_MESSAGE_BUFFER + render_idx,
            MINT_LOG_BUFFER_SIZE - render_idx,
            "|\n%s",
            __MINT_COLOR_RESET);

        mint_hook_write(S_LOG_MESSAGE_BUFFER, __MINT_MIN(render_idx, MINT_LOG_BUFFER_SIZE));
    }

    mint_hook_unlock();
}

static bool __mint_should_log(mint_id_t id, mint_level_e level)
{
    __MINT_UNUSED(id);

    MINT_RETURN_IF(level > s_global_level, false);

    return true;
}
#endif

#if MINT_API_LEVEL == MINT_API_LEVEL_ADVANCED
void mint_set_level(mint_id_t id, mint_level_e level)
{
    MINT_RETURN_VOID_IF(id != MINT_ID_GLOBAL && id >= MINT_LOG_ID_MAX);
    MINT_RETURN_VOID_IF(level < MINT_LEVEL_ALWAYS || MINT_LEVEL_TRACE < level);

    if (id == MINT_ID_GLOBAL)
    {
        s_global_level = level;
        return;
    }

    s_log_contexts[id].level = level;
}

void mint_init_log_contexts(const mint_log_ctx_t *contexts, size_t num_contexts)
{
    MINT_RETURN_VOID_IF(!contexts || num_contexts == 0);

    // NOTE(Caleb): Manually validate the configuration now so that we can skip checks later
    for (size_t id = 0; id < num_contexts; id++)
    {
        const mint_log_ctx_t *src_ctx  = contexts + id;
        mint_log_ctx_t       *dest_ctx = s_log_contexts + id;

        if (MINT_LEVEL_ALWAYS <= src_ctx->level && src_ctx->level <= MINT_LEVEL_TRACE)
        {
            dest_ctx->level = src_ctx->level;
        }

        strncpy(dest_ctx->tag, src_ctx->tag, MINT_MAX_TAG_LEN);
        dest_ctx->tag[MINT_MAX_TAG_LEN] = '\0';
    }
}

void __mint_log_impl(
    mint_id_t id, mint_level_e level, char *file, int line, const char *format, ...)
{
    MINT_RETURN_VOID_IF(!__mint_should_log(id, level));

    file            = __mint_basename(file);
    const char *tag = (id == MINT_ID_GLOBAL) ? MINT_GLOBAL_TAG : s_log_contexts[id].tag;

#if MINT_ENABLE_TIME
    char time_str[__MINT_TIME_STR_LEN + 1];
    __mint_format_time(mint_hook_get_time(), time_str, __MINT_TIME_STR_LEN + 1);

    const char *log_format_format = "%s%s %s | %s | %s:%d | %s%s\n";
    int         log_format_size   = snprintf(
        NULL,
        0,
        log_format_format,
        S_LEVEL_COLORS[level],
        time_str,
        tag,
        S_LEVEL_STRINGS[level],
        file,
        line,
        format,
        __MINT_COLOR_RESET);
    MINT_RETURN_VOID_IF(log_format_size < 0);

    char log_format[log_format_size + 1];
    (void)snprintf(
        log_format,
        log_format_size + 1,
        log_format_format,
        S_LEVEL_COLORS[level],
        time_str,
        tag,
        S_LEVEL_STRINGS[level],
        file,
        line,
        format,
        __MINT_COLOR_RESET);
#else
    const char *log_format_format = "%s%s | %s | %s:%d | %s%s\n";
    int         log_format_size   = snprintf(
        NULL,
        0,
        log_format_format,
        S_LEVEL_COLORS[level],
        tag,
        S_LEVEL_STRINGS[level],
        file,
        line,
        format,
        __MINT_COLOR_RESET);
    MINT_RETURN_VOID_IF(log_format_size < 0);

    char log_format[log_format_size + 1];
    (void)snprintf(
        log_format,
        log_format_size + 1,
        log_format_format,
        S_LEVEL_COLORS[level],
        tag,
        S_LEVEL_STRINGS[level],
        file,
        line,
        format,
        __MINT_COLOR_RESET);
#endif

    mint_hook_lock();

    va_list args;
    va_start(args, format);

    int log_message_size = vsnprintf(S_LOG_MESSAGE_BUFFER, MINT_LOG_BUFFER_SIZE, log_format, args);

    va_end(args);

    if (log_message_size < 0)
    {
        mint_hook_unlock();
        return;
    }

    log_message_size = __MINT_MIN(log_message_size, MINT_LOG_BUFFER_SIZE - 1);
    mint_hook_write(S_LOG_MESSAGE_BUFFER, log_message_size);

    mint_hook_unlock();
}

void __mint_log_hex_impl(
    mint_id_t    id,
    mint_level_e level,
    char        *file,
    int          line,
    const char  *header,
    const void  *data,
    size_t       size)
{
    MINT_RETURN_VOID_IF(!__mint_should_log(id, level));

    file            = __mint_basename(file);
    const char *tag = (id == MINT_ID_GLOBAL) ? MINT_GLOBAL_TAG : s_log_contexts[id].tag;

    const char *line_header_format = NULL;
    int         line_header_size   = 0;

#if MINT_ENABLE_TIME
    char time_str[__MINT_TIME_STR_LEN + 1];
    __mint_format_time(mint_hook_get_time(), time_str, __MINT_TIME_STR_LEN + 1);

    line_header_format = (header) ? "%s%s %s | %s | %s:%d | %s | " : "%s%s %-6s | %s:%d | ";
    if (header)
    {
        line_header_size = snprintf(
            NULL,
            0,
            line_header_format,
            S_LEVEL_COLORS[level],
            time_str,
            tag,
            S_LEVEL_STRINGS[level],
            file,
            line,
            header);
    }
    else
    {
        line_header_size = snprintf(
            NULL,
            0,
            line_header_format,
            S_LEVEL_COLORS[level],
            time_str,
            tag,
            S_LEVEL_STRINGS[level],
            file,
            line);
    }

    // TODO(Caleb): Reconsider how to implement this without VLAs to have support for MSVC and C23
    MINT_RETURN_VOID_IF(line_header_size < 0);
    char line_header[line_header_size + 1];

    if (header)
    {
        snprintf(
            line_header,
            line_header_size + 1,
            line_header_format,
            S_LEVEL_COLORS[level],
            time_str,
            tag,
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
            S_LEVEL_COLORS[level],
            time_str,
            tag,
            S_LEVEL_STRINGS[level],
            file,
            line);
    }
#else
    line_header_format = (header) ? "%s%s | %s | %s:%d | %s | " : "%s%-6s | %s:%d | ";
    if (header)
    {
        line_header_size = snprintf(
            NULL,
            0,
            line_header_format,
            S_LEVEL_COLORS[level],
            tag,
            S_LEVEL_STRINGS[level],
            file,
            line,
            header);
    }
    else
    {
        line_header_size = snprintf(
            NULL,
            0,
            line_header_format,
            S_LEVEL_COLORS[level],
            tag,
            S_LEVEL_STRINGS[level],
            file,
            line);
    }

    // TODO(Caleb): Reconsider how to implement this without VLAs to have support for MSVC and C23
    MINT_RETURN_VOID_IF(line_header_size < 0);
    char line_header[line_header_size + 1];

    if (header)
    {
        snprintf(
            line_header,
            line_header_size + 1,
            line_header_format,
            S_LEVEL_COLORS[level],
            tag,
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
            S_LEVEL_COLORS[level],
            tag,
            S_LEVEL_STRINGS[level],
            file,
            line);
    }
#endif

    mint_hook_lock();

    for (size_t i = 0; i < size; i += 16)
    {
        int render_idx = line_header_size;

        memcpy(S_LOG_MESSAGE_BUFFER, line_header, line_header_size);

        // Line header + address ---------------------------

        render_idx += snprintf(
            S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "0x%08zX  ", i);

        // Hex bytes ---------------------------------------

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx,
                    MINT_LOG_BUFFER_SIZE - render_idx,
                    "%02X ",
                    ((const uint8_t *)data)[i + j]);
            }
            else
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "   ");
            }

            if (j == 7)
            {
                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, " ");
            }
        }

        // ASCII representation ----------------------------

        render_idx +=
            snprintf(S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "|");

        for (size_t j = 0; j < 16; j++)
        {
            if (i + j < size)
            {
                char c = ((const char *)data)[i + j];
                if (c < 32 || c > 126)
                {
                    c = '.';
                }

                render_idx += snprintf(
                    S_LOG_MESSAGE_BUFFER + render_idx, MINT_LOG_BUFFER_SIZE - render_idx, "%c", c);
            }
        }

        render_idx += snprintf(
            S_LOG_MESSAGE_BUFFER + render_idx,
            MINT_LOG_BUFFER_SIZE - render_idx,
            "|\n%s",
            __MINT_COLOR_RESET);

        mint_hook_write(S_LOG_MESSAGE_BUFFER, __MINT_MIN(render_idx, MINT_LOG_BUFFER_SIZE));
    }

    mint_hook_unlock();
}

static bool __mint_should_log(mint_id_t id, mint_level_e level)
{
    MINT_RETURN_IF(id != MINT_ID_GLOBAL && id >= MINT_LOG_ID_MAX, false);
    MINT_RETURN_IF(level < MINT_LEVEL_ALWAYS || MINT_LEVEL_TRACE < level, false);

    if (id == MINT_ID_GLOBAL)
    {
        return level <= s_global_level;
    }

    return level <= s_log_contexts[id].level;
}
#endif

// Private Helper Implementation -------------------------------------------------------------------

static char *__mint_basename(char *filename)
{
    char *fslash = strrchr(filename, '/');
    char *bslash = strrchr(filename, '\\');

    char *basename = (fslash > bslash) ? fslash : bslash;
    return basename ? basename + 1 : filename;
}

#if MINT_ENABLE_TIME
static void __mint_format_time(mint_time_t time, char *o_data, size_t size)
{
    uint32_t hours = time / (1000 * 60 * 60);
    time -= hours * (1000 * 60 * 60);
    uint32_t minutes = time / (1000 * 60);
    time -= minutes * (1000 * 60);
    uint32_t seconds = time / 1000;
    time -= seconds * 1000;

    snprintf(o_data, size, __MINT_TIME_FORMAT, hours, minutes, seconds, time);
}
#endif