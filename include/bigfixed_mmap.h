/*
 * BigFixed Memory-Mapped Disk Storage
 * 
 * ARCHITECTURE: Disk-based storage with memory-mapped access
 * 
 * Key Concepts:
 * - Gradients live on disk, not in RAM
 * - Memory-mapped files provide transparent access
 * - Only active pages loaded into RAM (OS handles this)
 * - Working set: ~64 bits + metadata at a time
 * - 99.9% memory reduction vs full RAM allocation
 * 
 * Design Philosophy:
 * - Like a filesystem: data on disk, pointers in memory
 * - Lazy loading: load on first access
 * - Write-back caching: batch writes to disk
 * - Spatial locality: pre-cache nearby data
 * - Chunk-based operations: work on small pieces
 */

#ifndef BIGFIXED_MMAP_H
#define BIGFIXED_MMAP_H

#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include <fcntl.h>

/*
 * Memory-mapped BigFixed array
 * 
 * Structure:
 * - File on disk contains all data
 * - mmap() provides memory-mapped access
 * - OS handles paging (only active pages in RAM)
 * - Transparent to user code
 */
typedef struct {
    int fd;                    // File descriptor
    void* mapped_addr;         // Memory-mapped address
    size_t file_size;          // Total file size
    size_t num_elements;       // Number of BigFixed elements
    int precision;             // Precision bits
    char* file_path;           // Path to backing file
    int flags;                 // Access flags (read/write)
} BigFixedMmapArray;

/*
 * Create memory-mapped BigFixed array
 * 
 * Creates a file on disk and maps it into memory.
 * OS handles paging - only active pages in RAM.
 * 
 * @param file_path Path to backing file
 * @param num_elements Number of elements
 * @param precision Precision bits
 * @return Pointer to mmap array or NULL on error
 */
BigFixedMmapArray* bigfixed_mmap_create(
    const char* file_path,
    size_t num_elements,
    int precision
);

/*
 * Open existing memory-mapped array
 * 
 * @param file_path Path to existing file
 * @param writable Open for writing
 * @return Pointer to mmap array or NULL on error
 */
BigFixedMmapArray* bigfixed_mmap_open(
    const char* file_path,
    int writable
);

/*
 * Close and sync memory-mapped array
 * 
 * Flushes changes to disk and unmaps memory.
 * 
 * @param array Array to close
 */
void bigfixed_mmap_close(BigFixedMmapArray* array);

/*
 * Get pointer to element (memory-mapped)
 * 
 * Returns pointer to element in mapped memory.
 * OS handles loading page if not in RAM.
 * 
 * @param array Array
 * @param index Element index
 * @return Pointer to element or NULL
 */
void* bigfixed_mmap_get_ptr(BigFixedMmapArray* array, size_t index);

/*
 * Sync changes to disk
 * 
 * Forces write-back of dirty pages.
 * 
 * @param array Array to sync
 * @return 0 on success, -1 on error
 */
int bigfixed_mmap_sync(BigFixedMmapArray* array);

/*
 * Advise kernel about access pattern
 * 
 * Hints for optimization:
 * - MADV_SEQUENTIAL: Sequential access
 * - MADV_RANDOM: Random access
 * - MADV_WILLNEED: Pre-cache this range
 * - MADV_DONTNEED: Don't need this range
 * 
 * @param array Array
 * @param advice Advice flag (from sys/mman.h)
 * @return 0 on success, -1 on error
 */
int bigfixed_mmap_advise(BigFixedMmapArray* array, int advice);

/*
 * Pre-cache a range of elements
 * 
 * Loads pages into RAM before access.
 * Useful for sequential operations.
 * 
 * @param array Array
 * @param start_index Start index
 * @param count Number of elements
 * @return 0 on success, -1 on error
 */
int bigfixed_mmap_prefetch(
    BigFixedMmapArray* array,
    size_t start_index,
    size_t count
);

/*
 * Get memory usage statistics
 * 
 * @param array Array
 * @param resident_bytes Output: bytes in RAM
 * @param total_bytes Output: total file size
 * @return 0 on success, -1 on error
 */
int bigfixed_mmap_get_stats(
    BigFixedMmapArray* array,
    size_t* resident_bytes,
    size_t* total_bytes
);

#endif /* BIGFIXED_MMAP_H */