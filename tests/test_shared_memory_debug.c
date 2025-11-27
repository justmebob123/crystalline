#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "shared_memory.h"

int main() {
    printf("Starting shared memory tests...\n");
    
    // Test 1: Basic creation
    printf("\nTest 1: Basic creation\n");
    SharedMemoryRegion* region = shared_memory_create(1024, SHARED_READ_WRITE);
    if (!region) {
        printf("FAILED: Could not create region\n");
        return 1;
    }
    printf("PASSED: Region created\n");
    
    // Test 2: Write data
    printf("\nTest 2: Write data\n");
    void* ptr = shared_memory_get_write_ptr(region);
    if (!ptr) {
        printf("FAILED: Could not get write pointer\n");
        return 1;
    }
    strcpy(ptr, "Test data");
    printf("PASSED: Data written\n");
    
    // Test 3: Read data
    printf("\nTest 3: Read data\n");
    const void* read_ptr = shared_memory_get_read_ptr(region);
    if (!read_ptr) {
        printf("FAILED: Could not get read pointer\n");
        return 1;
    }
    printf("Read: %s\n", (char*)read_ptr);
    printf("PASSED: Data read\n");
    
    // Test 4: COW mode
    printf("\nTest 4: COW mode\n");
    SharedMemoryRegion* cow_region = shared_memory_create(1024, SHARED_COPY_ON_WRITE);
    if (!cow_region) {
        printf("FAILED: Could not create COW region\n");
        return 1;
    }
    
    void* cow_ptr = shared_memory_get_write_ptr(cow_region);
    if (!cow_ptr) {
        printf("FAILED: Could not get COW write pointer\n");
        return 1;
    }
    strcpy(cow_ptr, "COW data");
    printf("PASSED: COW write\n");
    
    // Test 5: Statistics
    printf("\nTest 5: Statistics\n");
    SharedMemoryStats stats;
    shared_memory_get_stats(cow_region, &stats);
    printf("Reads: %lu, Writes: %lu, Copies: %lu\n", 
           stats.read_count, stats.write_count, stats.copy_count);
    printf("PASSED: Statistics retrieved\n");
    
    // Cleanup
    printf("\nCleaning up...\n");
    shared_memory_destroy(region);
    shared_memory_destroy(cow_region);
    
    printf("\nAll tests passed!\n");
    return 0;
}
