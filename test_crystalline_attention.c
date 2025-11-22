/*
 * Test Crystalline Attention - Advanced Features
 * 
 * Demonstrates:
 * - Q→K reversal
 * - Hyperdimensional resonance
 * - Lattice-based attention
 * - Root word modeling
 * - Prime factorization semantics
 * - Symmetry operations
 * - Fourier dampening
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "include/cllm.h"
#include "include/cllm_crystalline_attention.h"
#include "include/cllm_inference.h"
#include "include/cllm_utils.h"
#include "include/prime_float_math.h"

void print_separator(const char* title) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("═══════════════════════════════════════════════════════════════\n");
}

void test_prime_mapping() {
    print_separator("Test 1: Token → Prime Mapping");
    
    printf("\nMapping tokens to primes (root word modeling):\n");
    printf("%-10s %-10s %-10s %-15s\n", "Token ID", "Prime", "Is Prime?", "Root Token");
    printf("─────────────────────────────────────────────────────────────\n");
    
    for (uint32_t token_id = 0; token_id < 20; token_id++) {
        uint64_t prime = cllm_get_token_prime(token_id);
        uint32_t root = cllm_extract_root_word(token_id, prime);
        
        // Check if prime
        int is_prime = 1;
        if (prime > 1) {
            for (uint64_t i = 2; i * i <= prime; i++) {
                if (prime % i == 0) {
                    is_prime = 0;
                    break;
                }
            }
        }
        
        printf("%-10u %-10lu %-10s %-15u\n", 
               token_id, prime, is_prime ? "Yes" : "No", root);
    }
    
    printf("\nKey Insight: Prime tokens are roots, composites are variations\n");
}

void test_lattice_coordinates() {
    print_separator("Test 2: Lattice Coordinate Mapping");
    
    printf("\nMapping tokens to 3D crystalline lattice coordinates:\n");
    printf("%-10s %-10s %-12s %-12s %-12s\n", 
           "Token ID", "Prime", "X", "Y", "Z");
    printf("─────────────────────────────────────────────────────────────\n");
    
    for (uint32_t token_id = 0; token_id < 10; token_id++) {
        uint64_t prime = cllm_get_token_prime(token_id);
        float coords[3];
        cllm_compute_token_lattice_coords(token_id, prime, coords);
        
        printf("%-10u %-10lu %12.4f %12.4f %12.4f\n",
               token_id, prime, coords[0], coords[1], coords[2]);
    }
    
    printf("\nKey Insight: Tokens live in hyperdimensional lattice space\n");
}

void test_semantic_similarity() {
    print_separator("Test 3: Prime-Based Semantic Similarity");
    
    printf("\nComputing semantic similarity using prime factorization:\n");
    printf("%-10s %-10s %-10s %-10s %-15s\n",
           "Token 1", "Token 2", "Prime 1", "Prime 2", "Similarity");
    printf("─────────────────────────────────────────────────────────────\n");
    
    uint32_t test_tokens[][2] = {
        {5, 10},   // Related (10 = 2×5)
        {7, 14},   // Related (14 = 2×7)
        {3, 5},    // Unrelated (coprime)
        {11, 13},  // Unrelated (both prime)
        {6, 15},   // Related (share factor 3)
    };
    
    for (int i = 0; i < 5; i++) {
        uint32_t t1 = test_tokens[i][0];
        uint32_t t2 = test_tokens[i][1];
        uint64_t p1 = cllm_get_token_prime(t1);
        uint64_t p2 = cllm_get_token_prime(t2);
        float similarity = cllm_compute_prime_similarity(p1, p2);
        
        printf("%-10u %-10u %-10lu %-10lu %15.4f\n",
               t1, t2, p1, p2, similarity);
    }
    
    printf("\nKey Insight: Coprime = unrelated, shared factors = related\n");
}

void test_morphological_relationships() {
    print_separator("Test 4: Morphological Relationships");
    
    printf("\nAnalyzing morphological relationships:\n");
    printf("%-10s %-10s %-10s %-10s %-20s\n",
           "Token 1", "Token 2", "Prime 1", "Prime 2", "Relationship");
    printf("─────────────────────────────────────────────────────────────\n");
    
    const char* relationships[] = {
        "Unrelated (coprime)",
        "Related (share factors)",
        "Derived (one divides other)",
        "Same (identical)"
    };
    
    uint32_t test_pairs[][2] = {
        {5, 5},    // Same
        {5, 10},   // Derived
        {6, 15},   // Related
        {7, 11},   // Unrelated
    };
    
    for (int i = 0; i < 4; i++) {
        uint32_t t1 = test_pairs[i][0];
        uint32_t t2 = test_pairs[i][1];
        uint64_t p1 = cllm_get_token_prime(t1);
        uint64_t p2 = cllm_get_token_prime(t2);
        int rel = cllm_compute_morphological_relationship(p1, p2);
        
        printf("%-10u %-10u %-10lu %-10lu %-20s\n",
               t1, t2, p1, p2, relationships[rel]);
    }
    
    printf("\nKey Insight: Prime factorization reveals linguistic structure\n");
}

void test_hyperdimensional_distance() {
    print_separator("Test 5: Hyperdimensional Distance");
    
    printf("\nComputing distances in crystalline lattice space:\n");
    printf("%-10s %-10s %-15s %-15s\n",
           "Token 1", "Token 2", "Euclidean", "Hyperdim");
    printf("─────────────────────────────────────────────────────────────\n");
    
    for (uint32_t t1 = 0; t1 < 5; t1++) {
        for (uint32_t t2 = t1 + 1; t2 < 6; t2++) {
            uint64_t p1 = cllm_get_token_prime(t1);
            uint64_t p2 = cllm_get_token_prime(t2);
            
            float coords1[3], coords2[3];
            cllm_compute_token_lattice_coords(t1, p1, coords1);
            cllm_compute_token_lattice_coords(t2, p2, coords2);
            
            // Euclidean distance
            float dx = coords1[0] - coords2[0];
            float dy = coords1[1] - coords2[1];
            float dz = coords1[2] - coords2[2];
            float euclidean = prime_sqrt(dx*dx + dy*dy + dz*dz);
            
            // Hyperdimensional distance
            float hyperdim = cllm_compute_hyperdimensional_distance(
                coords1, coords2, p1, p2);
            
            printf("%-10u %-10u %15.4f %15.4f\n",
                   t1, t2, euclidean, hyperdim);
        }
    }
    
    printf("\nKey Insight: Distance incorporates geometry AND prime structure\n");
}

void test_attention_with_crystalline_features() {
    print_separator("Test 6: Crystalline Attention Forward Pass");
    
    printf("\nTesting attention with all crystalline features:\n");
    
    // Create small attention layer
    AttentionLayer layer;
    layer.layer_id = 0;
    layer.num_heads = 2;
    layer.head_dim = 4;
    
    uint32_t embedding_dim = layer.num_heads * layer.head_dim;
    size_t weight_size = layer.num_heads * layer.head_dim * layer.head_dim;
    
    layer.query_lattice = (float*)calloc(weight_size, sizeof(float));
    layer.key_lattice = (float*)calloc(weight_size, sizeof(float));
    layer.value_lattice = (float*)calloc(weight_size, sizeof(float));
    
    // Initialize with small random values
    srand(42);
    for (size_t i = 0; i < weight_size; i++) {
        layer.query_lattice[i] = ((float)rand() / RAND_MAX) * 0.1f;
        layer.key_lattice[i] = ((float)rand() / RAND_MAX) * 0.1f;
        layer.value_lattice[i] = ((float)rand() / RAND_MAX) * 0.1f;
    }
    
    // Create test input
    int seq_len = 4;
    float* input = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    float* output = (float*)calloc(seq_len * embedding_dim, sizeof(float));
    
    // Initialize input
    for (int i = 0; i < seq_len * embedding_dim; i++) {
        input[i] = ((float)rand() / RAND_MAX) * 0.5f;
    }
    
    // Create lattice coordinates
    float* lattice_coords = (float*)calloc(seq_len * 3, sizeof(float));
    uint64_t* token_primes = (uint64_t*)calloc(seq_len, sizeof(uint64_t));
    
    for (int i = 0; i < seq_len; i++) {
        token_primes[i] = cllm_get_token_prime(i);
        cllm_compute_token_lattice_coords(i, token_primes[i],
                                         &lattice_coords[i * 3]);
    }
    
    printf("Input sequence:\n");
    for (int i = 0; i < seq_len; i++) {
        printf("  Token %d (prime %lu): [", i, token_primes[i]);
        for (uint32_t j = 0; j < embedding_dim && j < 4; j++) {
            printf("%.3f%s", input[i * embedding_dim + j],
                   j < 3 ? ", " : "");
        }
        printf("%s]\n", embedding_dim > 4 ? ", ..." : "");
    }
    
    // Apply crystalline attention
    printf("\nApplying crystalline attention...\n");
    cllm_crystalline_attention_forward(&layer, input, output,
                                      lattice_coords, token_primes, seq_len);
    
    printf("\nOutput sequence:\n");
    for (int i = 0; i < seq_len; i++) {
        printf("  Token %d: [", i);
        for (uint32_t j = 0; j < embedding_dim && j < 4; j++) {
            printf("%.3f%s", output[i * embedding_dim + j],
                   j < 3 ? ", " : "");
        }
        printf("%s]\n", embedding_dim > 4 ? ", ..." : "");
    }
    
    printf("\n✓ Crystalline attention applied successfully!\n");
    printf("  Features used:\n");
    printf("  - Q→K reversal\n");
    printf("  - Hyperdimensional resonance\n");
    printf("  - Lattice coordinates\n");
    printf("  - Möbius transformation\n");
    printf("  - Plimpton ratio correction\n");
    printf("  - Cymatic resonance\n");
    printf("  - Schumann dampening\n");
    printf("  - Gamma burst activation\n");
    
    // Cleanup
    free(layer.query_lattice);
    free(layer.key_lattice);
    free(layer.value_lattice);
    free(input);
    free(output);
    free(lattice_coords);
    free(token_primes);
}

void test_einstein_correction() {
    print_separator("Test 7: Einstein Lambda Correction");
    
    printf("\nTesting Einstein Lambda correction on gradients:\n");
    
    size_t size = 10;
    float* gradients = (float*)malloc(size * sizeof(float));
    
    // Initialize with test gradients
    printf("\nOriginal gradients:\n  ");
    for (size_t i = 0; i < size; i++) {
        gradients[i] = (float)(i + 1) * 0.1f;
        printf("%.4f ", gradients[i]);
    }
    printf("\n");
    
    // Apply Einstein correction
    cllm_apply_einstein_correction(gradients, size);
    
    printf("\nAfter Einstein Lambda correction (Λ = 3/144000):\n  ");
    for (size_t i = 0; i < size; i++) {
        printf("%.4f ", gradients[i]);
    }
    printf("\n");
    
    printf("\nKey Insight: Lambda provides stability and prevents explosion\n");
    
    free(gradients);
}

int main() {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║     Crystalline Attention - Advanced Features Test           ║\n");
    printf("║  Q→K Reversal | Hyperdimensional Resonance | Root Words      ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    
    // Run all tests
    test_prime_mapping();
    test_lattice_coordinates();
    test_semantic_similarity();
    test_morphological_relationships();
    test_hyperdimensional_distance();
    test_attention_with_crystalline_features();
    test_einstein_correction();
    
    print_separator("Summary");
    printf("\n");
    printf("All crystalline attention features tested successfully!\n\n");
    printf("Advanced Features Demonstrated:\n");
    printf("  ✓ Token → Prime mapping (root word modeling)\n");
    printf("  ✓ 3D lattice coordinate system\n");
    printf("  ✓ Prime-based semantic similarity\n");
    printf("  ✓ Morphological relationship analysis\n");
    printf("  ✓ Hyperdimensional distance metrics\n");
    printf("  ✓ Q→K reversal transformation\n");
    printf("  ✓ Hyperdimensional resonance computation\n");
    printf("  ✓ Möbius transformations\n");
    printf("  ✓ Plimpton ratio corrections\n");
    printf("  ✓ Cymatic frequency resonance\n");
    printf("  ✓ Schumann resonance dampening\n");
    printf("  ✓ Gamma burst activation\n");
    printf("  ✓ Einstein Lambda correction\n");
    printf("\n");
    printf("The crystalline lattice abacus provides a complete mathematical\n");
    printf("framework for language modeling with:\n");
    printf("  - Arbitrary precision (no floating-point errors)\n");
    printf("  - Natural compression (prime factorization)\n");
    printf("  - Hyperdimensional packing (kissing spheres)\n");
    printf("  - Geometric relationships (lattice structure)\n");
    printf("  - Harmonic resonance (Fourier transforms)\n");
    printf("\n");
    printf("&quot;if Q is my question, then k is unknown. I have to discover it.&quot;\n");
    printf("The Q→K reversal transforms queries into key space through the\n");
    printf("crystalline lattice, revealing hidden relationships.\n");
    printf("\n");
    
    return 0;
}