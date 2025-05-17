# `mint`
MINT Is Not Testing! - Logging and debugging facilities for C

## Synopsis

`mint` provides a set of macros and functions for structured, leveled, and optionally subsystem-tagged logging in C.  
It supports log filtering by level and subsystem, colored output, time-stamping, hex dumps, and assertion/return helpers.  

## Features

- **Multiple API levels:**  
  - *Barebones*: Minimal, always-on logging macros.
  - *Simple*: Global log level filtering.
  - *Advanced*: Per-subsystem log levels and tags.
- **Log levels:** Always, Notify, Fatal, Error, Warn, Info, Debug, Trace.
- **Subsystem IDs and tags** (Advanced API only).
- **Hex dump macros** for binary data.
- **Conditional logging** and assertion macros.
- **Return-if and return-log-if helpers**.
- **Colored output** (configurable).
- **Timestamping** (configurable, user-supplied time hook).
- **Thread-safety hooks** (user-supplied).
- **Customizable output** (user-supplied write hook).

## Library Usage

To get started, include the `mint.h` and `mint.c` files in your project. To configure `mint`, see the configuration macros
at the top of `mint.h`. The header contains documentation for the public APIs and hooks, and the examples in the `examples`
directory show how the library can be used.

### API Level Selection

`mint` includes support for three different API levels:

- `MINT_API_LEVEL_BAREBONES`: Minimal, no levels or IDs.
- `MINT_API_LEVEL_SIMPLE`: Global log level filtering.
- `MINT_API_LEVEL_ADVANCED`: Per-ID log levels and tags.

### Log Levels

`mint` defines the following log levels for usage within the user's application:

| Level  | Macro     | Description            |
| ------ | --------- | ---------------------- |
| ALWAYS | MINT_LOG  | Always log             |
| NOTIFY | MINT_LOGN | Important notification |
| FATAL  | MINT_LOGF | Fatal error            |
| ERROR  | MINT_LOGE | Error                  |
| WARN   | MINT_LOGW | Warning                |
| INFO   | MINT_LOGI | Informational          |
| DEBUG  | MINT_LOGD | Debugging              |
| TRACE  | MINT_LOGT | Trace/verbose          |

### Hooks

This library is designed to be able to be used in embedded system and desktop systems alike, and so it provides a hook function
that may be implemented for a specific platform to provide log output. By default, the implementation uses `printf`.
(see: `void mint_hook_write(const char *str, size_t size)`)

As a provider of assertion and forced crash macros, the library provides a hook function to allow the user to specify what should
happen when the logic is forces into either state. The library will log the error, but it is up to the user to provide any other
handling that they would like (system reset, saving data, etc.). (see: `void mint_hook_on_assert_failed(void)`)

`mint` is not thread safe by default, but provides hooks that the user may implement in order to implement mutual exclusion. If these
hooks are not implemented, `mint` can make no guarantees about its thread safety. (see: `void mint_hook_lock(void)` / `void mint_hook_unlock(void)`)

If the uesr would like to display the current system time in front of each log message, they must not only enable the proper configuration
option, but they will also have to implement the hook function that will return the current system time (or uptime). The type of this
timestamp may also be changed, though it must be an integer type which is cast-able to a `uint32_t`. (see: `mint_time_t mint_hook_get_time(void)`)

- `mint_hook_get_time(void)` — For timestamps (default: returns 0).

### Examples

To see usage examples at the different API levels, see the `mintex_*.c` programs in the `examples/` directory.


