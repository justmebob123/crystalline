/**
 * Production Configuration for Crystalline CLLM
 * Optimized settings for production deployment
 */

#ifndef PRODUCTION_CONFIG_H
#define PRODUCTION_CONFIG_H

// Production build flags
#define PRODUCTION_BUILD 1
#define ENABLE_LOGGING 1
#define ENABLE_PROFILING 0
#define ENABLE_DEBUG_CHECKS 0

// Performance optimization settings
#define USE_LATTICE_EMBEDDINGS 1
#define USE_NTT_ATTENTION 1
#define USE_CYMATIC_TRAINING 1
#define USE_KISSING_SPHERES 1

// Memory management
#define MAX_MEMORY_POOL_SIZE (1024 * 1024 * 1024)  // 1GB
#define ENABLE_MEMORY_POOLING 1
#define ENABLE_GARBAGE_COLLECTION 1

// Threading configuration
#define MAX_THREADS 8
#define ENABLE_PARALLEL_PROCESSING 1

// Model size limits
#define MAX_VOCAB_SIZE 100000
#define MAX_EMBED_DIM 2048
#define MAX_SEQ_LENGTH 32768
#define MAX_NUM_LAYERS 48
#define MAX_NUM_HEADS 32

// Training configuration
#define DEFAULT_BATCH_SIZE 32
#define DEFAULT_LEARNING_RATE 0.0001f
#define DEFAULT_DROPOUT 0.1f
#define DEFAULT_WEIGHT_DECAY 0.01f

// Lattice configuration
#define DEFAULT_LATTICE_DIMENSION 8
#define DEFAULT_KISSING_NUMBER 24
#define DEFAULT_LAMBDA 1.0f

// Cymatic configuration
#define DEFAULT_BASE_FREQUENCY 432.0f
#define DEFAULT_HARMONIC_COUNT 8
#define DEFAULT_RESONANCE_THRESHOLD 0.7f
#define DEFAULT_PHASE_SHIFT 0.0f

// NTT configuration
#define NTT_MODULUS 998244353
#define NTT_PRIMITIVE_ROOT 3
#define NTT_MAX_SIZE 262144  // 2^18

// Checkpoint configuration
#define ENABLE_CHECKPOINTING 1
#define CHECKPOINT_INTERVAL 1000
#define MAX_CHECKPOINTS 5

// Validation configuration
#define VALIDATION_INTERVAL 100
#define EARLY_STOPPING_PATIENCE 10
#define MIN_DELTA 0.0001f

// Error handling
#define MAX_RETRY_ATTEMPTS 3
#define ENABLE_GRACEFUL_DEGRADATION 1

// Logging levels
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_CRITICAL = 4
} LogLevel;

#define DEFAULT_LOG_LEVEL LOG_LEVEL_INFO

// Production deployment settings
typedef struct {
    // Model configuration
    int vocab_size;
    int embed_dim;
    int num_heads;
    int num_layers;
    int max_seq_len;
    
    // Training configuration
    float learning_rate;
    float dropout;
    float weight_decay;
    int batch_size;
    
    // Mathematical framework
    int use_lattice_embeddings;
    int use_ntt_attention;
    int use_cymatic_training;
    int lattice_dimension;
    int kissing_number;
    float lambda;
    float base_frequency;
    int harmonic_count;
    
    // Performance settings
    int num_threads;
    int enable_profiling;
    size_t memory_limit;
    
    // Checkpoint settings
    int enable_checkpointing;
    int checkpoint_interval;
    const char* checkpoint_dir;
    
    // Logging settings
    LogLevel log_level;
    const char* log_file;
    
} ProductionConfig;

// Default production configuration
static inline ProductionConfig get_default_production_config() {
    ProductionConfig config = {
        // Model configuration
        .vocab_size = 50000,
        .embed_dim = 512,
        .num_heads = 8,
        .num_layers = 12,
        .max_seq_len = 2048,
        
        // Training configuration
        .learning_rate = DEFAULT_LEARNING_RATE,
        .dropout = DEFAULT_DROPOUT,
        .weight_decay = DEFAULT_WEIGHT_DECAY,
        .batch_size = DEFAULT_BATCH_SIZE,
        
        // Mathematical framework
        .use_lattice_embeddings = USE_LATTICE_EMBEDDINGS,
        .use_ntt_attention = USE_NTT_ATTENTION,
        .use_cymatic_training = USE_CYMATIC_TRAINING,
        .lattice_dimension = DEFAULT_LATTICE_DIMENSION,
        .kissing_number = DEFAULT_KISSING_NUMBER,
        .lambda = DEFAULT_LAMBDA,
        .base_frequency = DEFAULT_BASE_FREQUENCY,
        .harmonic_count = DEFAULT_HARMONIC_COUNT,
        
        // Performance settings
        .num_threads = MAX_THREADS,
        .enable_profiling = ENABLE_PROFILING,
        .memory_limit = MAX_MEMORY_POOL_SIZE,
        
        // Checkpoint settings
        .enable_checkpointing = ENABLE_CHECKPOINTING,
        .checkpoint_interval = CHECKPOINT_INTERVAL,
        .checkpoint_dir = "./checkpoints",
        
        // Logging settings
        .log_level = DEFAULT_LOG_LEVEL,
        .log_file = "./cllm_production.log"
    };
    
    return config;
}

#endif // PRODUCTION_CONFIG_H