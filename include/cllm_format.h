#ifndef CLLM_FORMAT_H
#define CLLM_FORMAT_H

/*
 * cllm_format.h - Auto-generated header file
 * Source: cllm_format.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "prime_types.h"

/* Local includes */
#include "cllm.h"

/* Function declarations */
void cllm_header_init(CLLMHeader* header, const char* model_name, const char* description);
void cllm_prime_to_lattice(uint64_t prime, float coords[3], float* angle, float* radius);
uint64_t cllm_nearest_prime(uint64_t n);
uint64_t cllm_token_to_prime(uint32_t token_id);
void cllm_token_create(CLLMToken* token, uint32_t token_id, const char* token_str);
void cllm_lattice_point_create(CLLMLatticePoint* point, uint32_t point_id, 
                               float x, float y, float z, uint32_t prime);
float cllm_lattice_distance(const CLLMLatticePoint* p1, const CLLMLatticePoint* p2);
void cllm_lattice_find_neighbors(CLLMLatticePoint* point, CLLMLatticePoint* all_points, 
                                 int num_points, float max_distance);
/**
 * MODEL I/O FUNCTIONS
 * 
 * RECOMMENDED API (use these):
 *   cllm_read_model()  - Load model with proper validation
 *   cllm_write_model() - Save model with layer weights
 * 
 * DEPRECATED API (legacy compatibility):
 *   cllm_read()  - Old format, use cllm_read_model() instead
 *   cllm_write() - Old format, use cllm_write_model() instead
 */

// RECOMMENDED: New API with proper layer-by-layer save/load
CLLMModel* cllm_read_model(const char* filename);
int cllm_write_model(const CLLMModel* model, const char* filepath);

// DEPRECATED: Old API for backward compatibility
CLLMModel* cllm_read(const char* filename);
int cllm_write(const char* filename, CLLMModel* model);

// Forward declaration for validation
int cllm_validate_model(void* model);

// Additional format functions
int cllm_write_checkpoint(const char* filename, CLLMModel* model);
CLLMModel* cllm_read_checkpoint(const char* filename);
bool cllm_validate(const char* filename);
void cllm_free(CLLMModel* model);

#endif /* CLLM_FORMAT_H */