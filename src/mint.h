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

/// Select the API level to use. Exactly one API level must be selected. The behavior of the
/// implementation is undefined if more than one API level is selected, or if an API level is set to
/// anything other than 0 or 1.

/// MINT_BAREBONES_API: The lowest API level with the most basic features. No levels, no IDs, no
///                     fuss. You get basic logging macros as well as all of the conditional logging
///                     macros and assertion macros. Recommended for small, single-task projects
///                     which do not need to make use of logging levels.
#define MINT_BAREBONES_API   1

/// MINT_SIMPLE_API:    A more fully featured, yet still simple API level that allows the
///                     programmer to associate a level with each log message, which allows
///                     filtering to decide which messages are actually output to the screen.
///                     Recommended for substantial projects that output a lot of debugging
///                     information, but do not need the flexibility of logging IDs.
#define MINT_SIMPLE_API      0

/// MINT_ADVANCED_API:  The highest API level with the most features. This allows the
///                     programmer to associate a level and an ID with each log message, which
///                     allows the implementation to not only perform filtering based on levels,
///                     but also to mask entire subsystems from logging based on their ID.
///                     Recommended for large, multi-task projects with different subsystems
///                     in which it may be useful to silence particular subsystems.
#define MINT_ADVANCED_API    0

/// If set to zero, the implementation will obtain the length of the log message buffer by doing a
/// no-op format on the log message. It will then allocate a buffer of the resulting size on the
/// stack and format the message into it. This ensures that no log messages are truncated, but may
/// incur a (relatively small) performance penalty due to varargs needing to be evaluated and
/// formatted twice.
///
/// If nonzero, the implementation will use a statically allocated buffer of the specified size for
/// all log messages, which may improve performance, but can also result in message truncation. The
/// behavior of library functions is undefined for cases where this buffer size is too small.
#define MINT_LOG_BUFFER_SIZE 0

// Hooks -------------------------------------------------------------------------------------------

/// @brief Hook for writing to the console. Defaults to using `printf`.
/// @param[in] str The string to write.
/// @param[in] size The length of the string.
__MINT_WEAK void mint_hook_write(const char *str, size_t size);

/// @brief Hook for when an assertion fails. Defaults to spinning forever.
__MINT_WEAK void mint_hook_on_assert_failed(void);

// Public API --------------------------------------------------------------------------------------

// NOTE(Caleb):
// If a user takes the time to define these in such a way that this condition is satisfied, each
// value isn't a one or a zero, and the library compiles, then they deserve the undefined behavior
// that will result.
#if MINT_BAREBONES_API + MINT_SIMPLE_API + MINT_ADVANCED_API != 1
#error "Exactly one of MINT_BAREBONES_API, MINT_SIMPLE_API, and MINT_ADVANCED_API must be set"
#endif

#if MINT_SIMPLE_API || MINT_ADVANCED_API
#error "MINT_SIMPLE_API and MINT_ADVANCED_API are not yet implemented"
#endif

#if MINT_BAREBONES_API

/// @brief Log a formatted message to the console.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_LOG(__fmt, ...) __mint_log_impl_simple(__FILE__, __LINE__, __fmt, ##__VA_ARGS__)

/// @brief Conditionally log a formatted message to the console.
/// @param[in] __cond The condition to check.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_LOG_IF(__cond, __fmt, ...)                                                            \
    do                                                                                             \
    {                                                                                              \
        if (!!(__cond))                                                                            \
        {                                                                                          \
            MINT_LOG(__fmt, ##__VA_ARGS__);                                                        \
        }                                                                                          \
    } while (0)

/// @brief Log a hex dump to the console.
/// @param[in] __header The header to print before the hex dump.
/// @param[in] __data The data to dump.
/// @param[in] __size The number of bytes to dump.
#define MINT_LOG_HEX(__header, __data, __size)                                                     \
    __mint_log_hex_impl_simple(__FILE__, __LINE__, (__header), (__data), (__size))

/// @brief Assert that a condition is true, log a formatted error message if it is not.
/// @param[in] __cond The condition to check.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_CHECK(__cond, __fmt, ...)                                                             \
    do                                                                                             \
    {                                                                                              \
        if (!(__cond))                                                                             \
        {                                                                                          \
            MINT_LOG("Check failed: '%s' - "__fmt, (#__cond), ##__VA_ARGS__);                      \
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
            MINT_LOG("Assertion failed: '%s' - "__fmt, (#__cond), ##__VA_ARGS__);                  \
            mint_hook_on_assert_failed();                                                          \
        }                                                                                          \
    } while (0)

#endif

/// @brief Conditionally log a formatted message to the console and return a value.
/// @param[in] __cond The condition to check.
/// @param[in] __retval The value to return.
/// @param[in] __fmt The printf-style format string for the message.
/// @param[in] ... The arguments for the format string.
#define MINT_RETURN_LOG_IF(__cond, __retval, __fmt, ...)                                           \
    do                                                                                             \
    {                                                                                              \
        if (!!(__cond))                                                                            \
        {                                                                                          \
            MINT_LOG(__fmt, ##__VA_ARGS__);                                                        \
            return __retval;                                                                       \
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
        MINT_LOG("Crash forced! " __fmt, ##__VA_ARGS__);                                           \
        mint_hook_on_assert_failed();                                                              \
    } while (0)

// Implementation Details --------------------------------------------------------------------------

void __mint_log_impl_simple(char *file, int line, const char *format, ...) __MINT_PRINTFLIKE(3, 4);
void __mint_log_hex_impl_simple(
    char *file, int line, const char *header, const void *data, size_t size);
