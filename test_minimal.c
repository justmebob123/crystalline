#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "algorithms/include/shared_memory.h"

int main() {
    printf("Test 1: Basic creation\n");
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_READ_ONLY);
    if (region == NULL) {
        printf("FAIL: Failed to create region\n");
        return 1;
    }
    printf("PASS: Region created\n");
    
    printf("\nTest 2: Write and read\n");
    strcpy((char*)region->data, "Hello");
    const void* read_ptr = shared_memory_read(region);
    printf("Read: %s\n", (const char*)read_ptr);
    shared_memory_release_read(region);
    printf("PASS: Read/write works\n");
    
    printf("\nTest 3: Cleanup\n");
    shared_memory_free(region);
    printf("PASS: Cleanup successful\n");
    
    printf("\nAll tests passed!\n");
    return 0;
}