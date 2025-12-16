#ifndef CLLM_SHARED_MEMORY_H
#define CLLM_SHARED_MEMORY_H

/**
 * CLLM Shared Memory - Wrapper for Algorithm Library
 * 
 * This header now uses the algorithm library's shared memory implementation.
 * All types and functions are provided by the algorithm library.
 * 
 * The algorithm library provides:
 * - SharedMemoryAccessMode enum (READ_ONLY, COPY_ON_WRITE, LOCKED_WRITE)
 * - SharedMemoryRegion structure
 * - All shared memory functions
 * 
 * CLLM-specific extensions can be added below if needed.
 */

// Use algorithm library's shared memory implementation
#include "../../algorithms/include/shared_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

// CLLM-specific shared memory extensions can be added here
// Currently, all functionality is provided by the algorithm library

#ifdef __cplusplus
}
#endif

#endif /* CLLM_SHARED_MEMORY_H */