#include "../algorithms/include/shared_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("Test 1: Basic Creation and Destruction\n");
    
    // Test READ_ONLY creation
    SharedMemoryRegion* region1 = shared_memory_create(1024, SHARED_READ_ONLY);
    if (!region1) {
        printf("FAILED: Could not create READ_ONLY region\n");
        return 1;
    }
    printf("  READ_ONLY region created: size=%zu, mode=%d\n", region1->size, region1->access_mode);
    shared_memory_free(region1);
    printf("  READ_ONLY region freed\n");
    
    // Test COPY_ON_WRITE creation
    SharedMemoryRegion* region2 = shared_memory_create(2048, SHARED_COPY_ON_WRITE);
    if (!region2) {
        printf("FAILED: Could not create COPY_ON_WRITE region\n");
        return 1;
    }
    printf("  COPY_ON_WRITE region created: size=%zu, mode=%d\n", region2->size, region2->access_mode);
    shared_memory_free(region2);
    printf("  COPY_ON_WRITE region freed\n");
    
    // Test LOCKED_WRITE creation
    SharedMemoryRegion* region3 = shared_memory_create(4096, SHARED_LOCKED_WRITE);
    if (!region3) {
        printf("FAILED: Could not create LOCKED_WRITE region\n");
        return 1;
    }
    printf("  LOCKED_WRITE region created: size=%zu, mode=%d\n", region3->size, region3->access_mode);
    shared_memory_free(region3);
    printf("  LOCKED_WRITE region freed\n");
    
    printf("PASSED: Basic Creation and Destruction\n\n");
    
    printf("Test 2: Read-Only Access\n");
    SharedMemoryRegion* ro_region = shared_memory_create(1024, SHARED_READ_ONLY);
    if (!ro_region) {
        printf("FAILED: Could not create READ_ONLY region\n");
        return 1;
    }
    
    // Initialize data
    strcpy((char*)ro_region->data, "Read-only data");
    printf("  Data initialized: %s\n", (char*)ro_region->data);
    
    // Read data
    const void* read_ptr = shared_memory_read(ro_region);
    if (!read_ptr) {
        printf("FAILED: Could not read from READ_ONLY region\n");
        return 1;
    }
    printf("  Data read: %s\n", (char*)read_ptr);
    shared_memory_release_read(ro_region);
    
    shared_memory_free(ro_region);
    printf("PASSED: Read-Only Access\n\n");
    
    printf("Test 3: Copy-On-Write\n");
    SharedMemoryRegion* cow_region = shared_memory_create(1024, SHARED_COPY_ON_WRITE);
    if (!cow_region) {
        printf("FAILED: Could not create COPY_ON_WRITE region\n");
        return 1;
    }
    
    // Initialize data
    strcpy((char*)cow_region->data, "Original data");
    printf("  Initial data: %s\n", (char*)cow_region->data);
    printf("  is_copy before write: %d\n", cow_region->is_copy);
    
    // Trigger COW
    void* write_ptr = shared_memory_write(cow_region);
    if (!write_ptr) {
        printf("FAILED: Could not write to COPY_ON_WRITE region\n");
        return 1;
    }
    strcpy((char*)write_ptr, "Modified data");
    shared_memory_release_write(cow_region);
    
    printf("  is_copy after write: %d\n", cow_region->is_copy);
    printf("  Data after modification: %s\n", (char*)cow_region->data);
    
    // Get statistics
    uint64_t reads, writes, copies;
    shared_memory_get_stats(cow_region, &reads, &writes, &copies);
    printf("  Statistics: reads=%lu, writes=%lu, copies=%lu\n", reads, writes, copies);
    
    shared_memory_free(cow_region);
    printf("PASSED: Copy-On-Write\n\n");
    
    printf("All tests passed!\n");
    return 0;
}
