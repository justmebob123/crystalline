#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shared_memory.h"

int main() {
    printf("Test 1: Basic creation\n");
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_READ_ONLY);
    if (!region) {
        printf("FAILED\n");
        return 1;
    }
    printf("PASSED\n");
    
    printf("\nTest 2: Free region\n");
    shared_memory_free(region);
    printf("PASSED\n");
    
    printf("\nTest 3: COW creation\n");
    SharedMemoryRegion* cow = shared_memory_create(1024, SHARED_COPY_ON_WRITE);
    if (!cow) {
        printf("FAILED\n");
        return 1;
    }
    printf("PASSED\n");
    
    printf("\nTest 4: COW write\n");
    void* ptr = shared_memory_write(cow);
    if (!ptr) {
        printf("FAILED\n");
        return 1;
    }
    strcpy(ptr, "Test");
    shared_memory_release_write(cow);
    printf("PASSED\n");
    
    printf("\nTest 5: Free COW\n");
    shared_memory_free(cow);
    printf("PASSED\n");
    
    printf("\nAll tests passed!\n");
    return 0;
}
