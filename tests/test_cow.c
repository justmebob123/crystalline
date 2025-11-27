#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "algorithms/include/shared_memory.h"

int main() {
    printf("Test: Copy-On-Write\n");
    
    // Create COW region
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_COPY_ON_WRITE);
    if (region == NULL) {
        printf("Failed to create region\n");
        return 1;
    }
    
    printf("Region created\n");
    
    // Write initial data
    strcpy((char*)region->data, "Original data");
    printf("Initial data: %s\n", (char*)region->data);
    printf("is_copy before write: %d\n", region->is_copy);
    
    // Trigger COW by writing
    printf("\nTriggering COW...\n");
    void* write_ptr = shared_memory_write(region);
    if (write_ptr == NULL) {
        printf("Failed to get write pointer\n");
        shared_memory_free(region);
        return 1;
    }
    
    printf("Got write pointer\n");
    printf("is_copy after write: %d\n", region->is_copy);
    
    // Modify the data
    strcpy((char*)write_ptr, "Modified data");
    shared_memory_release_write(region);
    
    // Read back
    const void* read_ptr = shared_memory_read(region);
    printf("Data after modification: %s\n", (const char*)read_ptr);
    shared_memory_release_read(region);
    
    // Check statistics
    uint64_t reads, writes, copies;
    shared_memory_get_stats(region, &reads, &writes, &copies);
    printf("\nStatistics:\n");
    printf("Reads: %lu, Writes: %lu, Copies: %lu\n", reads, writes, copies);
    
    shared_memory_free(region);
    printf("\nTest passed!\n");
    
    return 0;
}