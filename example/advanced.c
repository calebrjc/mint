#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mint.h"

#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

// Define some log IDs for subsystems
enum
{
    LOGID_NET = 0,
    LOGID_IO,
    LOGID_APP,
    LOGID_DBG,
    LOGID_COUNT
};

static const mint_log_ctx_t S_LOG_CONTEXTS[LOGID_COUNT] = {
    [LOGID_NET] = {MINT_LEVEL_INFO, "NET"},
    [LOGID_IO]  = {MINT_LEVEL_DEBUG, "IO "},
    [LOGID_APP] = {MINT_LEVEL_WARN, "APP"},
    [LOGID_DBG] = {MINT_LEVEL_TRACE, "DBG"},
};

static const uint8_t S_TEST_DATA[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x10, 0x32, 0x54, 0x76,
    0x98, 0xBA, 0xDC, 0xFE, 0x00, 0xFF, 0xAA, 0x55, 0x5A, 0xA5, 0xC3, 0x3C, 0x7E, 0xE7, 0x1B, 0xB1,
    0x2D, 0xD2, 0x4C, 0xC4, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC,
    0xDD, 0xEE, 0xF0, 0x0F, 0x1E, 0x2D, 0x3C, 0x4B, 0x5A, 0x69, 0x78, 0x87, 0x96, 0xA5, 0xB4,
};

int test_return_log_if(int x)
{
    MINT_RETURN_LOG_IF(x < 0, -1, LOGID_NET, "Negative value: %d", x);
    MINT_RETURN_LOGN_IF(x == 0, -2, LOGID_IO, "Zero value: %d", x);
    MINT_RETURN_LOGF_IF(x == 42, -3, LOGID_APP, "The answer is 42!");
    MINT_RETURN_LOGE_IF(x > 100, -4, LOGID_DBG, "Too large: %d", x);
    MINT_RETURN_LOGW_IF(x == 1, -5, LOGID_NET, "x is one");
    MINT_RETURN_LOGI_IF(x == 2, -6, LOGID_IO, "x is two");
    MINT_RETURN_LOGD_IF(x == 3, -7, LOGID_APP, "x is three");
    MINT_RETURN_LOGT_IF(x == 4, -8, LOGID_DBG, "x is four");
    return 0;
}

void test_return_void_log_if(int x)
{
    MINT_RETURN_VOID_LOG_IF(x < 0, LOGID_NET, "Negative value: %d", x);
    MINT_RETURN_VOID_LOGN_IF(x == 0, LOGID_IO, "Zero value: %d", x);
    MINT_RETURN_VOID_LOGF_IF(x == 42, LOGID_APP, "The answer is 42!");
    MINT_RETURN_VOID_LOGE_IF(x > 100, LOGID_DBG, "Too large: %d", x);
    MINT_RETURN_VOID_LOGW_IF(x == 1, LOGID_NET, "x is one");
    MINT_RETURN_VOID_LOGI_IF(x == 2, LOGID_IO, "x is two");
    MINT_RETURN_VOID_LOGD_IF(x == 3, LOGID_APP, "x is three");
    MINT_RETURN_VOID_LOGT_IF(x == 4, LOGID_DBG, "x is four");
}

int main(void)
{
    // Initialize log contexts (per-ID levels and tags)
    mint_init_log_contexts(S_LOG_CONTEXTS, LOGID_COUNT);

    // Set global log level to DEBUG (default is DEBUG, but let's show changing it)
    mint_set_level(MINT_ID_GLOBAL, MINT_LEVEL_DEBUG);

    // Set per-ID log levels (override initial context)
    mint_set_level(LOGID_NET, MINT_LEVEL_INFO);
    mint_set_level(LOGID_IO, MINT_LEVEL_DEBUG);
    mint_set_level(LOGID_APP, MINT_LEVEL_WARN);
    mint_set_level(LOGID_DBG, MINT_LEVEL_TRACE);

    // Basic log macros (with IDs)
    MINT_LOG(LOGID_NET, "Network always log");
    MINT_LOGN(LOGID_IO, "IO notify log: %d", 123);
    MINT_LOGF(LOGID_APP, "App fatal log: %s", "something went wrong");
    MINT_LOGE(LOGID_DBG, "Debug error log: %f", 3.14);
    MINT_LOGW(LOGID_NET, "Network warning log: %s", "be careful!");
    MINT_LOGI(LOGID_IO, "IO info log: %s", "all systems nominal");
    MINT_LOGD(LOGID_APP, "App debug log: %x", 0xDEADBEEF);
    MINT_LOGT(LOGID_DBG, "Debug trace log: %p", (void *)S_TEST_DATA);

    // Log with global ID
    MINT_LOG(MINT_ID_GLOBAL, "Global always log");
    MINT_LOGN(MINT_ID_GLOBAL, "Global notify log");
    MINT_LOGF(MINT_ID_GLOBAL, "Global fatal log");
    MINT_LOGE(MINT_ID_GLOBAL, "Global error log");
    MINT_LOGW(MINT_ID_GLOBAL, "Global warning log");
    MINT_LOGI(MINT_ID_GLOBAL, "Global info log");
    MINT_LOGD(MINT_ID_GLOBAL, "Global debug log");
    MINT_LOGT(MINT_ID_GLOBAL, "Global trace log");

    // Lower the global log level to WARN, so INFO/DEBUG/TRACE will not print for global logs
    MINT_LOGI(
        MINT_ID_GLOBAL, "Setting global log level to WARN (INFO/DEBUG/TRACE will be filtered)");
    mint_set_level(MINT_ID_GLOBAL, MINT_LEVEL_WARN);

    MINT_LOG(MINT_ID_GLOBAL, "This should print (ALWAYS)");
    MINT_LOGN(MINT_ID_GLOBAL, "This should print (NOTIFY)");
    MINT_LOGF(MINT_ID_GLOBAL, "This should print (FATAL)");
    MINT_LOGE(MINT_ID_GLOBAL, "This should print (ERROR)");
    MINT_LOGW(MINT_ID_GLOBAL, "This should print (WARN)");
    MINT_LOGI(MINT_ID_GLOBAL, "This should NOT print (INFO)");
    MINT_LOGD(MINT_ID_GLOBAL, "This should NOT print (DEBUG)");
    MINT_LOGT(MINT_ID_GLOBAL, "This should NOT print (TRACE)");

    // Restore global log level to DEBUG for rest of tests
    mint_set_level(MINT_ID_GLOBAL, MINT_LEVEL_DEBUG);

    // Conditional log macros (with IDs)
    MINT_LOG_IF(1, LOGID_NET, "This should always print");
    MINT_LOGN_IF(0, LOGID_IO, "This should NOT print");
    MINT_LOGF_IF(1, LOGID_APP, "Fatal condition met: %d", 1);
    MINT_LOGE_IF(0, LOGID_DBG, "Error condition NOT met");
    MINT_LOGW_IF(1, LOGID_NET, "Warning: %s", "condition true");
    MINT_LOGI_IF(0, LOGID_IO, "Info: should NOT print");
    MINT_LOGD_IF(1, LOGID_APP, "Debug: %d", 42);
    MINT_LOGT_IF(0, LOGID_DBG, "Trace: should NOT print");

    // Hex dump macros (with IDs)
    MINT_LOG_HEX(LOGID_NET, "Test Data NET ", S_TEST_DATA, ARRAYSIZE(S_TEST_DATA));
    MINT_LOGN_HEX(LOGID_IO, "Test Data IO  ", S_TEST_DATA, 16);
    MINT_LOGF_HEX(LOGID_APP, "Test Data APP ", S_TEST_DATA, 8);
    MINT_LOGE_HEX(LOGID_DBG, "Test Data DBG ", S_TEST_DATA, 4);
    MINT_LOGW_HEX(LOGID_NET, "Test Data Warn", S_TEST_DATA, 2);
    MINT_LOGI_HEX(LOGID_IO, "Test Data Info", S_TEST_DATA, 1);
    MINT_LOGD_HEX(LOGID_APP, "Test Data Dbg ", S_TEST_DATA, 0); // Should print header only
    MINT_LOGT_HEX(LOGID_DBG, "Test Data Trc ", S_TEST_DATA, ARRAYSIZE(S_TEST_DATA));

    // Return log if macros (with IDs)
    printf("test_return_log_if(-5) = %d\n", test_return_log_if(-5));
    printf("test_return_log_if(0) = %d\n", test_return_log_if(0));
    printf("test_return_log_if(42) = %d\n", test_return_log_if(42));
    printf("test_return_log_if(101) = %d\n", test_return_log_if(101));
    printf("test_return_log_if(1) = %d\n", test_return_log_if(1));
    printf("test_return_log_if(2) = %d\n", test_return_log_if(2));
    printf("test_return_log_if(3) = %d\n", test_return_log_if(3));
    printf("test_return_log_if(4) = %d\n", test_return_log_if(4));
    printf("test_return_log_if(5) = %d\n", test_return_log_if(5));

    // Return void log if macros (with IDs)
    test_return_void_log_if(-1);
    test_return_void_log_if(0);
    test_return_void_log_if(42);
    test_return_void_log_if(101);
    test_return_void_log_if(1);
    test_return_void_log_if(2);
    test_return_void_log_if(3);
    test_return_void_log_if(4);
    test_return_void_log_if(5);

    // Check and assert macros (global ID)
    MINT_CHECK(1 == 1, "This check should pass (no log)");
    MINT_CHECK(1 == 0, "This check should fail and log: %d", 123);

    // Uncomment to test assertion failure (will spin forever by default)
    // MINT_ASSERT(0, "This assert should fail and halt!");

    // Uncomment to test crash macro (will spin forever by default)
    // MINT_CRASH("This is a forced crash!");

    // Return macros
    MINT_RETURN_IF(0, EXIT_FAILURE); // Should not return
    MINT_RETURN_IF(1, EXIT_SUCCESS); // Should return here

    // This should not be reached
    MINT_LOG(LOGID_APP, "End of test program (should not print if previous return worked)");

    return 0;
}
