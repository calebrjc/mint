#pragma once

#include <stddef.h>

#ifdef __GNUC__
#define __MINT_WEAK                 __attribute__((weak))
#define __MINT_PRINTFLIKE(__x, __y) __attribute__((format(printf, __x, __y)))
#else
#define __MINT_WEAK
#define __MINT_PRINTFLIKE(...)

#endif
// Configuration -----------------------------------------------------------------------------------

// API Level Selection ---------------------------------------------------------

/// The lowest API level with the most basic features. No levels, no IDs, no fuss. You get basic
/// logging macros as well as all of the conditional logging macros and assertion macros.
/// Recommended for small, single-task projects which do not need to make use of logging levels.
#define MINT_API_LEVEL_BAREBONES 0

/// A more fully featured, yet still simple API level that allows the programmer to associate a
/// level with each log message, which allows filtering to decide which messages are actually output
/// to the screen. Recommended for substantial projects that output a lot of debugging information,
/// but do not need the flexibility of logging IDs.
#define MINT_API_LEVEL_SIMPLE    1

/// The highest API level with the most features. This allows the programmer to associate a level
/// and an ID with each log message, which allows the implementation to not only perform filtering
/// based on levels, but also to mask entire subsystems from logging based on their ID. Recommended
/// for large, multi-task projects with different subsystems in which it may be useful to silence
/// particular subsystems.
#define MINT_API_LEVEL_ADVANCED  2

// Select the API level to use. See the descriptions above for details.
#define MINT_API_LEVEL           MINT_API_LEVEL_SIMPLE

// Other Configuration Parameters ----------------------------------------------

/// Set the line buffer size for the logging implementation. If too small, messages which are
/// longer than this will be truncated.
///
/// NOTE: For LOG*_HEX macros, logs are written line by line, so the buffer should be large enough
/// to hold at least a line of hexdump output if used (this depends on the API level).
#define MINT_LOG_BUFFER_SIZE     128

/// Enable or disable colored output. Only affects MINT_API_LEVEL_SIMPLE and above.
#define MINT_ENABLE_COLORS       1

// Hooks -------------------------------------------------------------------------------------------

/// @brief Hook for writing to the console. Defaults to using `printf`.
/// @param[in] str The string to write.
/// @param[in] size The length of the string.
__MINT_WEAK void mint_hook_write(const char *str, size_t size);

/// @brief Hook for when an assertion fails. Defaults to spinning forever.
__MINT_WEAK void mint_hook_on_assert_failed(void);

// Public API --------------------------------------------------------------------------------------

#if MINT_API_LEVEL == MINT_API_LEVEL_ADVANCED
#error "MINT_API_LEVEL_ADVANCED is not implemented yet"
#endif

typedef int mint_id_t;
#define MINT_ID_GLOBAL ((mint_id_t) - 1)

typedef enum
{
    MINT_LEVEL_ALWAYS = 0,
    MINT_LEVEL_NOTIFY,
    MINT_LEVEL_FATAL,
    MINT_LEVEL_ERROR,
    MINT_LEVEL_WARN,
    MINT_LEVEL_INFO,
    MINT_LEVEL_DEBUG,
    MINT_LEVEL_TRACE,
} mint_level_e;

/// @brief Set a logging level.
/// @param id The ID to set the level for.
/// @param level The level to set.
/// @note For MINT_API_LEVEL_BAREBONES, this function does nothing.
/// @note For MINT_API_LEVEL_SIMPLE, this function sets the global level and should use
///       `MINT_ID_GLOBAL` for the `id` parameter.
void mint_set_level(mint_id_t id, mint_level_e level);

// LOG -------------------------------------------------------------------------

#define __MINT_LOG_IMPL(__level, __fmt, ...)                                                       \
    __mint_log_impl((__level), 0, (__FILE__), (__LINE__), (__fmt), ##__VA_ARGS__);

/// @brief Log a formatted message to the console.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_LOG(__fmt, ...) __MINT_LOG_IMPL(MINT_LEVEL_ALWAYS, (__fmt), ##__VA_ARGS__)

#if MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
#define MINT_LOGN(__fmt, ...) __MINT_LOG_IMPL(MINT_LEVEL_NOTIFY, (__fmt), ##__VA_ARGS__)
#define MINT_LOGF(__fmt, ...) __MINT_LOG_IMPL(MINT_LEVEL_FATAL, (__fmt), ##__VA_ARGS__)
#define MINT_LOGE(__fmt, ...) __MINT_LOG_IMPL(MINT_LEVEL_ERROR, (__fmt), ##__VA_ARGS__)
#define MINT_LOGW(__fmt, ...) __MINT_LOG_IMPL(MINT_LEVEL_WARN, (__fmt), ##__VA_ARGS__)
#define MINT_LOGI(__fmt, ...) __MINT_LOG_IMPL(MINT_LEVEL_INFO, (__fmt), ##__VA_ARGS__)
#define MINT_LOGD(__fmt, ...) __MINT_LOG_IMPL(MINT_LEVEL_DEBUG, (__fmt), ##__VA_ARGS__)
#define MINT_LOGT(__fmt, ...) __MINT_LOG_IMPL(MINT_LEVEL_TRACE, (__fmt), ##__VA_ARGS__)
#endif

// LOG_IF ----------------------------------------------------------------------

#define __MINT_LOG_IF_IMPL(__level, __cond, __fmt, ...)                                            \
    do                                                                                             \
    {                                                                                              \
        if (!!(__cond))                                                                            \
        {                                                                                          \
            __MINT_LOG_IMPL((__level), (__fmt), ##__VA_ARGS__);                                    \
        }                                                                                          \
    } while (0)

/// @brief Conditionally log a formatted message to the console.
/// @param[in] __cond The condition to check.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_LOG_IF(__cond, __fmt, ...)                                                            \
    __MINT_LOG_IF_IMPL(MINT_LEVEL_ALWAYS, (__cond), (__fmt), ##__VA_ARGS__)

#if MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
#define MINT_LOGN_IF(__cond, __fmt, ...)                                                           \
    __MINT_LOG_IF_IMPL(MINT_LEVEL_NOTIFY, (__cond), (__fmt), ##__VA_ARGS__)
#define MINT_LOGF_IF(__cond, __fmt, ...)                                                           \
    __MINT_LOG_IF_IMPL(MINT_LEVEL_FATAL, (__cond), (__fmt), ##__VA_ARGS__)
#define MINT_LOGE_IF(__cond, __fmt, ...)                                                           \
    __MINT_LOG_IF_IMPL(MINT_LEVEL_ERROR, (__cond), (__fmt), ##__VA_ARGS__)
#define MINT_LOGW_IF(__cond, __fmt, ...)                                                           \
    __MINT_LOG_IF_IMPL(MINT_LEVEL_WARN, (__cond), (__fmt), ##__VA_ARGS__)
#define MINT_LOGI_IF(__cond, __fmt, ...)                                                           \
    __MINT_LOG_IF_IMPL(MINT_LEVEL_INFO, (__cond), (__fmt), ##__VA_ARGS__)
#define MINT_LOGD_IF(__cond, __fmt, ...)                                                           \
    __MINT_LOG_IF_IMPL(MINT_LEVEL_DEBUG, (__cond), (__fmt), ##__VA_ARGS__)
#define MINT_LOGT_IF(__cond, __fmt, ...)                                                           \
    __MINT_LOG_IF_IMPL(MINT_LEVEL_TRACE, (__cond), (__fmt), ##__VA_ARGS__)
#endif

// LOG_HEX ---------------------------------------------------------------------

#define __MINT_LOG_HEX_IMPL(__level, __header, __data, __size)                                     \
    __mint_log_hex_impl((__level), 0, (__FILE__), (__LINE__), (__header), (__data), (__size));

/// @brief Log a hex dump to the console.
/// @param[in] __header The header to print before the hex dump.
/// @param[in] __data The data to dump.
/// @param[in] __size The number of bytes to dump.
#define MINT_LOG_HEX(__header, __data, __size)                                                     \
    __MINT_LOG_HEX_IMPL(MINT_LEVEL_ALWAYS, (__header), (__data), (__size))

#if MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
#define MINT_LOGN_HEX(__header, __data, __size)                                                    \
    __MINT_LOG_HEX_IMPL(MINT_LEVEL_NOTIFY, (__header), (__data), (__size))
#define MINT_LOGF_HEX(__header, __data, __size)                                                    \
    __MINT_LOG_HEX_IMPL(MINT_LEVEL_FATAL, (__header), (__data), (__size))
#define MINT_LOGE_HEX(__header, __data, __size)                                                    \
    __MINT_LOG_HEX_IMPL(MINT_LEVEL_ERROR, (__header), (__data), (__size))
#define MINT_LOGW_HEX(__header, __data, __size)                                                    \
    __MINT_LOG_HEX_IMPL(MINT_LEVEL_WARN, (__header), (__data), (__size))
#define MINT_LOGI_HEX(__header, __data, __size)                                                    \
    __MINT_LOG_HEX_IMPL(MINT_LEVEL_INFO, (__header), (__data), (__size))
#define MINT_LOGD_HEX(__header, __data, __size)                                                    \
    __MINT_LOG_HEX_IMPL(MINT_LEVEL_DEBUG, (__header), (__data), (__size))
#define MINT_LOGT_HEX(__header, __data, __size)                                                    \
    __MINT_LOG_HEX_IMPL(MINT_LEVEL_TRACE, (__header), (__data), (__size))
#endif

// RETURN_LOG_IF --------------------------------------------------------------

#define __MINT_RETURN_LOG_IF_IMPL(__level, __cond, __retval, __fmt, ...)                           \
    do                                                                                             \
    {                                                                                              \
        if (!!(__cond))                                                                            \
        {                                                                                          \
            __MINT_LOG_IMPL((__level), (__fmt), ##__VA_ARGS__);                                    \
            return __retval;                                                                       \
        }                                                                                          \
    } while (0)

/// @brief Conditionally log a formatted message to the console and return a value.
/// @param[in] __cond The condition to check.
/// @param[in] __retval The value to return.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_RETURN_LOG_IF(__cond, __retval, __fmt, ...)                                           \
    __MINT_RETURN_LOG_IF_IMPL(MINT_LEVEL_ALWAYS, (__cond), (__retval), (__fmt), ##__VA_ARGS__)

#if MINT_API_LEVEL == MINT_API_LEVEL_SIMPLE
#define MINT_RETURN_LOGN_IF(__cond, __retval, __fmt, ...)                                          \
    __MINT_RETURN_LOG_IF_IMPL(MINT_LEVEL_NOTIFY, (__cond), (__retval), (__fmt), ##__VA_ARGS__)
#define MINT_RETURN_LOGF_IF(__cond, __retval, __fmt, ...)                                          \
    __MINT_RETURN_LOG_IF_IMPL(MINT_LEVEL_FATAL, (__cond), (__retval), (__fmt), ##__VA_ARGS__)
#define MINT_RETURN_LOGE_IF(__cond, __retval, __fmt, ...)                                          \
    __MINT_RETURN_LOG_IF_IMPL(MINT_LEVEL_ERROR, (__cond), (__retval), (__fmt), ##__VA_ARGS__)
#define MINT_RETURN_LOGW_IF(__cond, __retval, __fmt, ...)                                          \
    __MINT_RETURN_LOG_IF_IMPL(MINT_LEVEL_WARN, (__cond), (__retval), (__fmt), ##__VA_ARGS__)
#define MINT_RETURN_LOGI_IF(__cond, __retval, __fmt, ...)                                          \
    __MINT_RETURN_LOG_IF_IMPL(MINT_LEVEL_INFO, (__cond), (__retval), (__fmt), ##__VA_ARGS__)
#define MINT_RETURN_LOGD_IF(__cond, __retval, __fmt, ...)                                          \
    __MINT_RETURN_LOG_IF_IMPL(MINT_LEVEL_DEBUG, (__cond), (__retval), (__fmt), ##__VA_ARGS__)
#define MINT_RETURN_LOGT_IF(__cond, __retval, __fmt, ...)                                          \
    __MINT_RETURN_LOG_IF_IMPL(MINT_LEVEL_TRACE, (__cond), (__retval), (__fmt), ##__VA_ARGS__)
#endif

// Checks, Asserts, and Returns ------------------------------------------------

/// @brief Assert that a condition is true, log a formatted error message if it is not.
/// @param[in] __cond The condition to check.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_CHECK(__cond, __fmt, ...)                                                             \
    do                                                                                             \
    {                                                                                              \
        if (!(__cond))                                                                             \
        {                                                                                          \
            __MINT_LOG_IMPL(                                                                       \
                MINT_LEVEL_WARN, "Check failed: '%s' - "__fmt, (#__cond), ##__VA_ARGS__);          \
        }                                                                                          \
    } while (0)

/// @brief Assert that a condition is true. Log a formatted error message and trigger an assertion
///        if it is not.
/// @param[in] __cond The condition to check.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_ASSERT(__cond, __fmt, ...)                                                            \
    do                                                                                             \
    {                                                                                              \
        if (!(__cond))                                                                             \
        {                                                                                          \
            __MINT_LOG_IMPL(                                                                       \
                MINT_LEVEL_FATAL, "Assert failed: '%s' - "__fmt, (#__cond), ##__VA_ARGS__);        \
            mint_hook_on_assert_failed();                                                          \
        }                                                                                          \
    } while (0)

/// @brief Conditionally return a value.
/// @param[in] __cond The condition to check.
/// @param[in] ... Optional; The value to return if the condition is true.
#define MINT_RETURN_IF(__cond, ...)                                                                \
    do                                                                                             \
    {                                                                                              \
        if (!!(__cond))                                                                            \
        {                                                                                          \
            return __VA_ARGS__;                                                                    \
        }                                                                                          \
    } while (0)

/// @brief Log a formatted message to the console and trigger an assertion.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_CRASH(__fmt, ...)                                                                     \
    do                                                                                             \
    {                                                                                              \
        __MINT_LOG_IMPL(MINT_LEVEL_FATAL, "Crash forced! " __fmt, ##__VA_ARGS__);                  \
        mint_hook_on_assert_failed();                                                              \
    } while (0)

// Implementation Details --------------------------------------------------------------------------

void __mint_log_impl(
    mint_level_e level, mint_id_t id, char *file, int line, const char *format, ...)
    __MINT_PRINTFLIKE(5, 6);

void __mint_log_hex_impl(
    mint_level_e level,
    mint_id_t    id,
    char        *file,
    int          line,
    const char  *header,
    const void  *data,
    size_t       size);
