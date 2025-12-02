/*
 * BigFixed Memory-Mapped Disk Storage Implementation
 * 
 * This implements disk-based storage with memory-mapped access.
 * The OS handles paging - only active data is in RAM.
 * 
 * Memory Efficiency:
 * - 22M elements × 16 bytes = 352 MB file on disk
 * - Only ~1-10 MB in RAM at any time (active working set)
 * - 99% memory reduction vs full RAM allocation
 */

#include "bigfixed_mmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

/*
 * Create memory-mapped BigFixed array
 */
BigFixedMmapArray* bigfixed_mmap_create(
    const char* file_path,
    size_t num_elements,
    int precision
) {
    if (!file_path || num_elements == 0) return NULL;
    
    // Allocate array structure
    BigFixedMmapArray* array = (BigFixedMmapArray*)calloc(1, sizeof(BigFixedMmapArray));
    if (!array) return NULL;
    
    // Calculate file size (16 bytes per element for packed format)
    array->file_size = num_elements * 16;  // 2 × int64_t per element
    array->num_elements = num_elements;
    array->precision = precision;
    array->file_path = strdup(file_path);
    
    // Create file
    array->fd = open(file_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (array->fd < 0) {
        fprintf(stderr, "Failed to create file %s: %s\n", file_path, strerror(errno));
        free(array->file_path);
        free(array);
        return NULL;
    }
    
    // Set file size
    if (ftruncate(array->fd, array->file_size) < 0) {
        fprintf(stderr, "Failed to set file size: %s\n", strerror(errno));
        close(array->fd);
        free(array->file_path);
        free(array);
        return NULL;
    }
    
    // Memory-map the file
    array->mapped_addr = mmap(
        NULL,                    // Let kernel choose address
        array->file_size,        // Size to map
        PROT_READ | PROT_WRITE,  // Read/write access
        MAP_SHARED,              // Changes written to file
        array->fd,               // File descriptor
        0                        // Offset
    );
    
    if (array->mapped_addr == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap file: %s\n", strerror(errno));
        close(array->fd);
        free(array->file_path);
        free(array);
        return NULL;
    }
    
    // Initialize to zero (optional - file is already zero)
    // memset(array->mapped_addr, 0, array->file_size);
    
    // Advise kernel about access pattern
    madvise(array->mapped_addr, array->file_size, MADV_RANDOM);
    
    printf("✓ Created disk-backed array: %s\n", file_path);
    printf("  File size: %.2f MB on disk\n", array->file_size / (1024.0 * 1024.0));
    printf("  Memory-mapped: OS will page in/out as needed\n");
    printf("  Expected RAM usage: ~1-10 MB (active working set)\n");
    
    return array;
}

/*
 * Open existing memory-mapped array
 */
BigFixedMmapArray* bigfixed_mmap_open(
    const char* file_path,
    int writable
) {
    if (!file_path) return NULL;
    
    // Allocate array structure
    BigFixedMmapArray* array = (BigFixedMmapArray*)calloc(1, sizeof(BigFixedMmapArray));
    if (!array) return NULL;
    
    array->file_path = strdup(file_path);
    
    // Open file
    int flags = writable ? O_RDWR : O_RDONLY;
    array->fd = open(file_path, flags);
    if (array->fd < 0) {
        fprintf(stderr, "Failed to open file %s: %s\n", file_path, strerror(errno));
        free(array->file_path);
        free(array);
        return NULL;
    }
    
    // Get file size
    struct stat st;
    if (fstat(array->fd, &st) < 0) {
        fprintf(stderr, "Failed to stat file: %s\n", strerror(errno));
        close(array->fd);
        free(array->file_path);
        free(array);
        return NULL;
    }
    
    array->file_size = st.st_size;
    array->num_elements = array->file_size / 16;  // 16 bytes per element
    
    // Memory-map the file
    int prot = writable ? (PROT_READ | PROT_WRITE) : PROT_READ;
    array->mapped_addr = mmap(
        NULL,
        array->file_size,
        prot,
        MAP_SHARED,
        array->fd,
        0
    );
    
    if (array->mapped_addr == MAP_FAILED) {
        fprintf(stderr, "Failed to mmap file: %s\n", strerror(errno));
        close(array->fd);
        free(array->file_path);
        free(array);
        return NULL;
    }
    
    return array;
}

/*
 * Close and sync memory-mapped array
 */
void bigfixed_mmap_close(BigFixedMmapArray* array) {
    if (!array) return;
    
    // Sync changes to disk
    if (array->mapped_addr && array->mapped_addr != MAP_FAILED) {
        msync(array->mapped_addr, array->file_size, MS_SYNC);
        munmap(array->mapped_addr, array->file_size);
    }
    
    if (array->fd >= 0) {
        close(array->fd);
    }
    
    free(array->file_path);
    free(array);
}

/*
 * Get pointer to element
 */
void* bigfixed_mmap_get_ptr(BigFixedMmapArray* array, size_t index) {
    if (!array || index >= array->num_elements) return NULL;
    
    // Calculate offset (16 bytes per element)
    size_t offset = index * 16;
    
    // Return pointer into mapped memory
    // OS will page in if not in RAM
    return (char*)array->mapped_addr + offset;
}

/*
 * Sync changes to disk
 */
int bigfixed_mmap_sync(BigFixedMmapArray* array) {
    if (!array || !array->mapped_addr) return -1;
    
    return msync(array->mapped_addr, array->file_size, MS_SYNC);
}

/*
 * Advise kernel about access pattern
 */
int bigfixed_mmap_advise(BigFixedMmapArray* array, int advice) {
    if (!array || !array->mapped_addr) return -1;
    
    return madvise(array->mapped_addr, array->file_size, advice);
}

/*
 * Pre-cache a range of elements
 */
int bigfixed_mmap_prefetch(
    BigFixedMmapArray* array,
    size_t start_index,
    size_t count
) {
    if (!array || start_index + count > array->num_elements) return -1;
    
    size_t offset = start_index * 16;
    size_t length = count * 16;
    
    // Advise kernel to load these pages
    return madvise(
        (char*)array->mapped_addr + offset,
        length,
        MADV_WILLNEED
    );
}

/*
 * Get memory usage statistics
 */
int bigfixed_mmap_get_stats(
    BigFixedMmapArray* array,
    size_t* resident_bytes,
    size_t* total_bytes
) {
    if (!array || !resident_bytes || !total_bytes) return -1;
    
    *total_bytes = array->file_size;
    
    // Get resident pages (pages actually in RAM)
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t num_pages = (array->file_size + page_size - 1) / page_size;
    
    unsigned char* vec = (unsigned char*)calloc(num_pages, 1);
    if (!vec) return -1;
    
    if (mincore(array->mapped_addr, array->file_size, vec) < 0) {
        free(vec);
        return -1;
    }
    
    // Count resident pages
    size_t resident_pages = 0;
    for (size_t i = 0; i < num_pages; i++) {
        if (vec[i] & 1) resident_pages++;
    }
    
    *resident_bytes = resident_pages * page_size;
    free(vec);
    
    return 0;
}