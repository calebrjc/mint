#include <stdio.h>
#include <stdlib.h>

#include "mint.h"

void mint_hook_on_assert_failed(void)
{
    printf("Do this instead!\n");
    exit(0);
}

int main(int argc, char **argv)
{
    MINT_LOG("Hello %d", 4);
    MINT_ASSERT(0, "I CANT MAKE IT (ec %d)", -6UL);
    return 0;
}