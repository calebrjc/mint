#pragma once

#ifdef __GNUC__
#define __MINT_WEAK                 __attribute__((weak))
#define __MINT_PRINTFLIKE(__x, __y) __attribute__((format(printf, __x, __y)))
#else
#define __MINT_WEAK
#define __MINT_PRINTFLIKE(...)
#endif

// Configuration -----------------------------------------------------------------------------------

/// If set to zero, the implementation will obtain the length of the log message buffer by doing a
/// no-op format on the log message. It will then allocate a buffer of the resulting size on the
/// stack and format the message into it. This ensures that no log messages are truncated, but may
/// incur a (relatively small) performance penalty due to varargs needing to be evaluated and
/// formatted twice.
///
/// If nonzero, the implementation will use a statically allocated buffer of the specified size for
/// all log messages, which may improve performance, but can also result in message truncation.
#define MINT_LOG_BUFFER_SIZE 0

// Hooks -------------------------------------------------------------------------------------------

__MINT_WEAK void mint_hook_on_assert_failed(void);

// Public API --------------------------------------------------------------------------------------

#define MINT_LOG(__fmt, ...) __mint_log_impl(__FILE__, __LINE__, __fmt, ##__VA_ARGS__)

#define MINT_LOG_IF(__cond, __fmt, ...)                                                            \
    do                                                                                             \
    {                                                                                              \
        if (!!(__cond))                                                                            \
        {                                                                                          \
            MINT_LOG(__fmt, ##__VA_ARGS__)                                                         \
        }                                                                                          \
    } while (0)

#define MINT_RETURN_LOG_IF(__cond, __retval, __fmt, ...)                                           \
    do                                                                                             \
    {                                                                                              \
        if (!!(__cond))                                                                            \
        {                                                                                          \
            MINT_LOG(__fmt, ##__VA_ARGS__);                                                        \
            return __retval;                                                                       \
        }                                                                                          \
    } while (0)

#define MINT_RETURN_IF(__cond, ...)                                                                \
    do                                                                                             \
    {                                                                                              \
        if (!!(__cond))                                                                            \
        {                                                                                          \
            return __VA_ARGS__;                                                                    \
        }                                                                                          \
    } while (0)

#define MINT_ASSERT(__cond, __fmt, ...)                                                            \
    do                                                                                             \
    {                                                                                              \
        if (!(__cond))                                                                             \
        {                                                                                          \
            MINT_LOG("Assertion failed: %s", #__cond);                                             \
            MINT_LOG(__fmt, ##__VA_ARGS__);                                                        \
            mint_hook_on_assert_failed();                                                          \
        }                                                                                          \
    } while (0)

#define MINT_CRASH(__fmt, ...)                                                                     \
    do                                                                                             \
    {                                                                                              \
        MINT_LOG("Crash forced! " __fmt, ##__VA_ARGS__);                                           \
        mint_hook_on_assert_failed();                                                              \
    } while (0)

// Implementation Details --------------------------------------------------------------------------

void __mint_log_impl(char *file, int line, const char *format, ...) __MINT_PRINTFLIKE(3, 4);
