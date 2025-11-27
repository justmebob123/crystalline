#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "algorithms/include/shared_memory.h"

int main() {
    printf("Test 1: Basic creation...\n");
    
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_READ_ONLY);
    if (region == NULL) {
        printf("Failed to create region\n");
        return 1;
    }
    
    printf("Region created successfully\n");
    printf("Size: %zu\n", region->size);
    printf("Access mode: %d\n", region->access_mode);
    
    printf("\nTest 2: Write initial data...\n");
    strcpy((char*)region->data, "Hello, World!");
    
    printf("\nTest 3: Read data...\n");
    const void* read_ptr = shared_memory_read(region);
    if (read_ptr == NULL) {
        printf("Failed to read\n");
        shared_memory_free(region);
        return 1;
    }
    
    printf("Read: %s\n", (const char*)read_ptr);
    shared_memory_release_read(region);
    
    printf("\nTest 4: Get statistics...\n");
    uint64_t reads, writes, copies;
    shared_memory_get_stats(region, &reads, &writes, &copies);
    printf("Reads: %lu, Writes: %lu, Copies: %lu\n", reads, writes, copies);
    
    shared_memory_free(region);
    printf("\nAll tests passed!\n");
    
    return 0;
}