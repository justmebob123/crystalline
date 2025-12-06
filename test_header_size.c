#include "include/cllm.h"
#include <stdio.h>

int main() {
    printf("CLLMHeader size: %zu bytes\n", sizeof(CLLMHeader));
    printf("CLLM_MAGIC: 0x%08X\n", CLLM_MAGIC);
    return 0;
}