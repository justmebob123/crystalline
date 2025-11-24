/*
 * CLLM - Crystalline Lattice Language Model
 * Core type definitions and structures
 */

#ifndef CLLM_H
#define CLLM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Magic number for CLLM file format
#define CLLM_MAGIC 0x434C4C4D  // "CLLM"
#define CLLM_VERSION 1

// Maximum sizes
#define MAX_MODEL_NAME 256
#define MAX_DESCRIPTION 1024
#define MAX_NEIGHBORS 12

/*
 * CLLM Header - File format header
 */
typedef struct {
    char magic[8];               // Magic string "CLLM\x01\x00\x00\x00"
    uint32_t version;            // Format version
    char model_name[MAX_MODEL_NAME];
    char description[MAX_DESCRIPTION];
    uint64_t vocab_size;         // Number of tokens
    uint64_t num_lattice_points; // Number of lattice points
    uint64_t embedding_dim;      // Embedding dimension
    uint64_t num_layers;         // Number of layers
    uint64_t created_timestamp;  // Creation time
    uint64_t modified_timestamp; // Last modification time
    uint32_t symmetry_order;     // Symmetry order for lattice
    uint32_t architecture;       // Architecture type (1 = Transformer)
    uint32_t num_heads;          // Number of attention heads
    uint32_t context_length;     // Maximum context length
    double golden_ratio;         // Golden ratio constant
    uint64_t timestamp;          // General timestamp
    uint64_t total_params;       // Total number of parameters
    uint8_t reserved[192];       // Reserved for future use
} CLLMHeader;

/*
 * CLLM Token - Token with prime encoding
 */
typedef struct {
    uint32_t token_id;           // Token ID
    uint64_t prime_encoding;     // Prime number encoding
    float lattice_coords[3];     // 3D lattice coordinates
    float angle;                 // Angular position
    float radius;                // Radial distance
    char token_str[64];          // Token string
    float frequency;             // Token frequency
    uint32_t symmetry_group;     // Symmetry group (0 to SYMMETRY_ORDER-1)
    float spiral_angle;          // Spiral angle in lattice
    float radial_distance;       // Radial distance from center
    uint8_t reserved[20];        // Reserved
} CLLMToken;

/*
 * CLLM Lattice Point - Point in the CLLM lattice (for file format)
 */
typedef struct {
    uint32_t point_id;           // Point ID
    uint64_t prime;              // Associated prime number
    uint64_t prime_factor;       // Prime factorization component
    float coords[3];             // 3D coordinates
    float angle;                 // Angular position
    float radius;                // Radial distance
    float resonance;             // Resonance value
    uint32_t neighbors[MAX_NEIGHBORS];  // Neighbor point IDs
    uint32_t neighbor_ids[MAX_NEIGHBORS];  // Alternative neighbor storage
    uint32_t num_neighbors;      // Number of neighbors
    uint32_t neighbor_count;     // Neighbor count (alternative)
    float* embedding;            // Embedding vector (dynamic dimension)
    uint32_t symmetry_group;     // Symmetry group (0 to SYMMETRY_ORDER-1)
    uint8_t reserved[48];        // Reserved
} CLLMLatticePoint;

/*
 * Embeddings - Token embeddings with lattice transformations
 */
typedef struct {
    uint32_t vocab_size;         // Vocabulary size
    uint32_t embedding_dim;      // Embedding dimension
    float* embeddings;           // Embedding matrix [vocab_size x embedding_dim]
    float* lattice_transform;    // Lattice transformation matrix
    float* inverse_transform;    // Inverse transformation matrix
} Embeddings;

/*
 * Attention Layer - Multi-head attention with lattice structure
 */
typedef struct {
    uint32_t layer_id;           // Layer identifier
    uint32_t num_heads;          // Number of attention heads
    uint32_t head_dim;           // Dimension per head
    float* query_lattice;        // Query weight lattice
    float* key_lattice;          // Key weight lattice
    float* value_lattice;        // Value weight lattice
} AttentionLayer;

/*
 * Feed-Forward Layer - Position-wise feed-forward network
 */
typedef struct {
    uint32_t layer_id;           // Layer identifier
    uint32_t input_dim;          // Input dimension
    uint32_t hidden_dim;         // Hidden layer dimension
    uint32_t output_dim;         // Output dimension
    float* w1_lattice;           // First weight matrix
    float* w2_lattice;           // Second weight matrix
    float* bias1;                // First bias vector
    float* bias2;                // Second bias vector
} FeedForwardLayer;

/*
 * CLLM Layer Normalization - Normalize layer outputs
 */
typedef struct {
    uint32_t layer_id;           // Layer identifier
    uint32_t dim;                // Dimension to normalize
    float epsilon;               // Small constant for numerical stability
    float* gamma;                // Scale parameters
    float* beta;                 // Shift parameters
} CLLMLayerNorm;

/*
 * Positional Encoding - Multiple encoding schemes
 */
typedef struct {
    uint32_t max_length;         // Maximum sequence length
    uint32_t embedding_dim;      // Embedding dimension
    float* spiral_positions;     // Spiral-based positions
    float* clock_positions;      // Clock-based positions
    float* prime_positions;      // Prime-based positions
    float* learned_positions;    // Learned positions
} PositionalEncoding;

/*
 * Lattice Embedding - Advanced lattice structure for embeddings
 */
typedef struct {
    uint32_t vocab_size;         // Vocabulary size
    uint32_t embedding_dim;      // Embedding dimension
    float* lattice_coords;       // 3D lattice coordinates [vocab_size x 3]
    float* lattice_transform;    // Lattice transformation matrix [dim x dim]
    float* inverse_transform;    // Inverse transformation [dim x dim]
    float* distance_matrix;      // Pairwise distances [vocab_size x vocab_size]
    uint32_t* nearest_neighbors; // K-nearest neighbors [vocab_size x k]
    uint32_t k_neighbors;        // Number of neighbors to track
} CLLMLatticeEmbedding;

/*
 * Symmetry Operations - D₁₂ dihedral group operations
 */
typedef struct {
    uint32_t d_model;            // Model dimension
    uint32_t num_rotations;      // Number of rotations (12)
    uint32_t num_reflections;    // Number of reflections (12)
    float* rotation_matrices;    // Rotation matrices [12 x d_model x d_model]
    float* reflection_matrices;  // Reflection matrices [12 x d_model x d_model]
    float* scaling_matrices;     // Scaling matrices [12 x d_model x d_model]
    float* invariant_features;   // Symmetry-invariant features
    uint32_t num_invariants;     // Number of invariant features
} CLLMSymmetryOps;

/*
 * CLLM Configuration - Model configuration parameters
 */
typedef struct {
    uint32_t vocab_size;         // Vocabulary size
    uint32_t embedding_dim;      // Embedding dimension
    uint32_t num_layers;         // Number of transformer layers
    uint32_t num_heads;          // Number of attention heads
    uint32_t ff_dim;             // Feed-forward dimension
    uint32_t max_seq_len;        // Maximum sequence length
    float dropout;               // Dropout rate
} CLLMConfig;

/*
 * Training Metadata - Training information
 */
typedef struct {
    uint64_t training_steps;     // Number of training steps
    float learning_rate;         // Learning rate used
    float loss;                  // Final loss value
    uint64_t timestamp;          // Training timestamp
    char optimizer[64];          // Optimizer name
    uint8_t reserved[128];       // Reserved for future use
} TrainingMetadata;

/*
 * CLLM Model - Complete model structure
 */
typedef struct {
    CLLMHeader header;           // Model header
    CLLMToken* tokens;           // Token array
    CLLMLatticePoint* lattice_points; // Lattice points
    uint64_t vocab_size;         // Vocabulary size
    uint64_t num_lattice_points; // Number of lattice points
    uint64_t embedding_dim;      // Embedding dimension
    float* weights;              // Model weights
    uint64_t num_weights;        // Number of weights
    
    // Embeddings
    Embeddings embeddings;       // Token embeddings with transformations
    
    // Transformer layers
    uint32_t num_layers;         // Number of transformer layers
    AttentionLayer* attention_layers;  // Attention layers
    FeedForwardLayer* ff_layers;       // Feed-forward layers
    CLLMLayerNorm* layer_norms;        // Layer normalization
    
    // Positional encoding
    PositionalEncoding pos_encoding;   // Positional encoding
    
    // Training metadata
    TrainingMetadata training_meta;    // Training information
} CLLMModel;

/*
 * Layer Normalization
 */
typedef struct {
    float* gamma;                // Scale parameters
    float* beta;                 // Shift parameters
    float epsilon;               // Small constant for stability
    uint32_t size;               // Layer size
} LayerNorm;

/*
 * Attention Head
 */
typedef struct {
    float* query_weights;        // Query projection weights
    float* key_weights;          // Key projection weights
    float* value_weights;        // Value projection weights
    float* output_weights;       // Output projection weights
    uint32_t head_dim;           // Dimension per head
} AttentionHead;

/*
 * CLLM Layer
 */
typedef struct {
    AttentionHead* attention_heads;  // Multi-head attention
    uint32_t num_heads;          // Number of attention heads
    LayerNorm* ln1;              // Layer norm 1
    LayerNorm* ln2;              // Layer norm 2
    float* ffn_weights1;         // Feed-forward network weights 1
    float* ffn_weights2;         // Feed-forward network weights 2
    uint32_t ffn_dim;            // Feed-forward dimension
} CLLMLayer;

#ifdef __cplusplus
}
#endif

#endif /* CLLM_H */