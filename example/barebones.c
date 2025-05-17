#include "mint.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

static const uint8_t S_TEST_DATA[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x10, 0x32, 0x54, 0x76,
    0x98, 0xBA, 0xDC, 0xFE, 0x00, 0xFF, 0xAA, 0x55, 0x5A, 0xA5, 0xC3, 0x3C, 0x7E, 0xE7, 0x1B, 0xB1,
    0x2D, 0xD2, 0x4C, 0xC4, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC,
    0xDD, 0xEE, 0xF0, 0x0F, 0x1E, 0x2D, 0x3C, 0x4B, 0x5A, 0x69, 0x78, 0x87, 0x96, 0xA5, 0xB4,
};

int test_return_log_if(int x)
{
    MINT_RETURN_LOG_IF(x < 0, -1, "Negative value: %d", x);
    MINT_RETURN_LOGN_IF(x == 0, -2, "Zero value: %d", x);
    MINT_RETURN_LOGF_IF(x == 42, -3, "The answer is 42!");
    MINT_RETURN_LOGE_IF(x > 100, -4, "Too large: %d", x);
    MINT_RETURN_LOGW_IF(x == 1, -5, "x is one");
    MINT_RETURN_LOGI_IF(x == 2, -6, "x is two");
    MINT_RETURN_LOGD_IF(x == 3, -7, "x is three");
    MINT_RETURN_LOGT_IF(x == 4, -8, "x is four");
    return 0;
}

void test_return_void_log_if(int x)
{
    MINT_RETURN_VOID_LOG_IF(x < 0, "Negative value: %d", x);
    MINT_RETURN_VOID_LOGN_IF(x == 0, "Zero value: %d", x);
    MINT_RETURN_VOID_LOGF_IF(x == 42, "The answer is 42!");
    MINT_RETURN_VOID_LOGE_IF(x > 100, "Too large: %d", x);
    MINT_RETURN_VOID_LOGW_IF(x == 1, "x is one");
    MINT_RETURN_VOID_LOGI_IF(x == 2, "x is two");
    MINT_RETURN_VOID_LOGD_IF(x == 3, "x is three");
    MINT_RETURN_VOID_LOGT_IF(x == 4, "x is four");
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    // Basic log macros
    MINT_LOG("Hello, Mint BAREBONES!");
    MINT_LOGN("Notify log: %d", 123);
    MINT_LOGF("Fatal log: %s", "something went wrong");
    MINT_LOGE("Error log: %f", 3.14);
    MINT_LOGW("Warning log: %s", "be careful!");
    MINT_LOGI("Info log: %s", "all systems nominal");
    MINT_LOGD("Debug log: %x", 0xDEADBEEF);
    MINT_LOGT("Trace log: %p", (void *)S_TEST_DATA);

    // Conditional log macros
    MINT_LOG_IF(1, "This should always print");
    MINT_LOGN_IF(0, "This should NOT print");
    MINT_LOGF_IF(1, "Fatal condition met: %d", 1);
    MINT_LOGE_IF(0, "Error condition NOT met");
    MINT_LOGW_IF(1, "Warning: %s", "condition true");
    MINT_LOGI_IF(0, "Info: should NOT print");
    MINT_LOGD_IF(1, "Debug: %d", 42);
    MINT_LOGT_IF(0, "Trace: should NOT print");

    // Hex dump macros
    MINT_LOG_HEX("Test Data", S_TEST_DATA, ARRAYSIZE(S_TEST_DATA));
    MINT_LOGN_HEX("Test Data Notify", S_TEST_DATA, 16);
    MINT_LOGF_HEX("Test Data Fatal", S_TEST_DATA, 8);
    MINT_LOGE_HEX("Test Data Error", S_TEST_DATA, 4);
    MINT_LOGW_HEX("Test Data Warn", S_TEST_DATA, 2);
    MINT_LOGI_HEX("Test Data Info", S_TEST_DATA, 1);
    MINT_LOGD_HEX("Test Data Debug", S_TEST_DATA, 0); // Should print header only
    MINT_LOGT_HEX("Test Data Trace", S_TEST_DATA, ARRAYSIZE(S_TEST_DATA));

    // Return log if macros
    printf("test_return_log_if(-5) = %d\n", test_return_log_if(-5));
    printf("test_return_log_if(0) = %d\n", test_return_log_if(0));
    printf("test_return_log_if(42) = %d\n", test_return_log_if(42));
    printf("test_return_log_if(101) = %d\n", test_return_log_if(101));
    printf("test_return_log_if(1) = %d\n", test_return_log_if(1));
    printf("test_return_log_if(2) = %d\n", test_return_log_if(2));
    printf("test_return_log_if(3) = %d\n", test_return_log_if(3));
    printf("test_return_log_if(4) = %d\n", test_return_log_if(4));
    printf("test_return_log_if(5) = %d\n", test_return_log_if(5));

    // Return void log if macros
    test_return_void_log_if(-1);
    test_return_void_log_if(0);
    test_return_void_log_if(42);
    test_return_void_log_if(101);
    test_return_void_log_if(1);
    test_return_void_log_if(2);
    test_return_void_log_if(3);
    test_return_void_log_if(4);
    test_return_void_log_if(5);

    // Check and assert macros
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
    MINT_LOG("End of test program (should not print if previous return worked)");

    return 0;
}
