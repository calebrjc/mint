#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mint.h"

#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))

uint32_t mint_hook_get_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int main(int argc, char **argv)
{
    int x = 5;

    mint_set_level(MINT_ID_GLOBAL, MINT_LEVEL_WARN);

    MINT_LOGN("Value of `x`: %d", x);
    MINT_LOG_IF(x == 0, "Value of `x` is zero");

    uint16_t values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    MINT_LOGI("Values size: %zu", ARRAYSIZE(values));
    MINT_LOGW_HEX("Values", values, sizeof(values));

    if (x == 5)
    {
        MINT_CRASH("Unexpected value of `x`: %d", x);
    }

    return 0;
}