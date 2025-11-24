/**
 * CLLM Crawler - Command Line Application
 * 
 * Continuous web crawling and training system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "cllm.h"
#include "cllm_training.h"

// Forward declarations from crawler modules
typedef struct CrawlerState CrawlerState;
typedef struct PreprocessorState PreprocessorState;
typedef struct TokenizerState TokenizerState;
typedef struct ContinuousTrainingState ContinuousTrainingState;

extern CrawlerState* crawler_init(const char* data_dir, const char* start_url, int max_pages);
extern void crawler_cleanup(CrawlerState* state);
extern void* crawler_thread_func(void* arg);

extern PreprocessorState* preprocessor_init(const char* data_dir);
extern void preprocessor_cleanup(PreprocessorState* state);
extern void* preprocessor_thread_func(void* arg);

extern TokenizerState* tokenizer_init(const char* data_dir);
extern void tokenizer_cleanup(TokenizerState* state);
extern void* tokenizer_thread_func(void* arg);

extern ContinuousTrainingState* continuous_training_init(const char* data_dir, const char* model_path, 
                                                          CLLMModel* model, int num_threads);
extern int continuous_training_start(ContinuousTrainingState* state, pthread_t* threads);
extern void continuous_training_stop(ContinuousTrainingState* state, pthread_t* threads);
extern void continuous_training_cleanup(ContinuousTrainingState* state);

// Global state for signal handling
static volatile int g_running = 1;

void signal_handler(int signum) {
    (void)signum;
    printf("\n\n=== SHUTDOWN SIGNAL RECEIVED ===\n");
    g_running = 0;
}

void print_usage(const char* prog) {
    printf("Usage: %s [OPTIONS]\n", prog);
    printf("\nOptions:\n");
    printf("  --start-url URL      Starting URL for crawler\n");
    printf("  --max-pages N        Maximum pages to crawl (default: 1000)\n");
    printf("  --data-dir PATH      Data directory (default: ~/.cllm_crawler)\n");
    printf("  --model-path PATH    Model file path (default: ~/.cllm_models/continuous_model.cllm)\n");
    printf("  --threads N          Number of training threads (default: CPU count - 3)\n");
    printf("  --help               Show this help\n");
    printf("\nExample:\n");
    printf("  %s --start-url https://example.com --max-pages 500\n", prog);
}

int main(int argc, char** argv) {
    // Default configuration
    char start_url[2048] = ""; // No default - must be provided via --start-url
    int max_pages = 1000;
    char data_dir[1024];
    char model_path[1024];
    int num_training_threads = 0;
    
    // Get home directory
    const char* home = getenv("HOME");
    if (!home) home = "/tmp";
    
    snprintf(data_dir, sizeof(data_dir), "%s/.cllm_crawler", home);
    snprintf(model_path, sizeof(model_path), "%s/.cllm_models/continuous_model.cllm", home);
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--start-url") == 0 && i + 1 < argc) {
            strncpy(start_url, argv[++i], sizeof(start_url) - 1);
        } else if (strcmp(argv[i], "--max-pages") == 0 && i + 1 < argc) {
            max_pages = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--data-dir") == 0 && i + 1 < argc) {
            strncpy(data_dir, argv[++i], sizeof(data_dir) - 1);
        } else if (strcmp(argv[i], "--model-path") == 0 && i + 1 < argc) {
            strncpy(model_path, argv[++i], sizeof(model_path) - 1);
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            num_training_threads = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Auto-detect thread count if not specified
    if (num_training_threads == 0) {
        long nproc = sysconf(_SC_NPROCESSORS_ONLN);
        num_training_threads = nproc - 3;  // Reserve 3 for crawler, preprocessor, tokenizer
        if (num_training_threads < 1) num_training_threads = 1;
    }
    
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("        CLLM CRAWLER - Continuous Learning System\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("Configuration:\n");
    printf("  Start URL: %s\n", start_url);
    printf("  Max pages: %d\n", max_pages);
    printf("  Data directory: %s\n", data_dir);
    printf("  Model path: %s\n", model_path);
    printf("  Training threads: %d\n", num_training_threads);
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create directories
    mkdir(data_dir, 0755);
    
    char models_dir[1024];
    snprintf(models_dir, sizeof(models_dir), "%s/.cllm_models", home);
    mkdir(models_dir, 0755);
    
    // Load or create model
    printf("=== Loading Model ===\n");
    CLLMModel* model = NULL;
    
    extern CLLMModel* cllm_read_model(const char* filepath);
    model = cllm_read_model(model_path);
    
    if (!model) {
        printf("No existing model found, creating new model...\n");
        
        // Create model configuration
        CLLMConfig config = {
            .vocab_size = 10000,
            .embedding_dim = 512,
            .num_layers = 6,
            .num_heads = 8,
            .ff_dim = 2048,
            .max_seq_len = 512,
            .dropout = 0.1f
        };
        
        extern CLLMModel* cllm_create_model(const CLLMConfig* config);
        model = cllm_create_model(&config);
        
        if (!model) {
            fprintf(stderr, "Failed to create model\n");
            return 1;
        }
        
        printf("✓ Model created\n");
    } else {
        printf("✓ Model loaded from: %s\n", model_path);
    }
    
    // Initialize components
    printf("\n=== Initializing Components ===\n");
    
    CrawlerState* crawler = crawler_init(data_dir, start_url, max_pages);
    PreprocessorState* preprocessor = preprocessor_init(data_dir);
    TokenizerState* tokenizer = tokenizer_init(data_dir);
    ContinuousTrainingState* training = continuous_training_init(data_dir, model_path, model, num_training_threads);
    
    if (!crawler || !preprocessor || !tokenizer || !training) {
        fprintf(stderr, "Failed to initialize components\n");
        return 1;
    }
    
    printf("✓ All components initialized\n");
    
    // Start threads
    printf("\n=== Starting Threads ===\n");
    
    pthread_t crawler_thread;
    pthread_t preprocessor_thread;
    pthread_t tokenizer_thread;
    pthread_t* training_threads = (pthread_t*)malloc(num_training_threads * sizeof(pthread_t));
    
    pthread_create(&crawler_thread, NULL, crawler_thread_func, crawler);
    pthread_create(&preprocessor_thread, NULL, preprocessor_thread_func, preprocessor);
    pthread_create(&tokenizer_thread, NULL, tokenizer_thread_func, tokenizer);
    continuous_training_start(training, training_threads);
    
    printf("✓ All threads started\n");
    printf("\n=== System Running ===\n");
    printf("Press Ctrl+C to stop\n\n");
    
    // Status monitoring loop
    while (g_running) {
        sleep(10);
        
        // Print status (access state safely)
        printf("\n=== STATUS UPDATE ===\n");
        printf("System running... (Ctrl+C to stop)\n");
        printf("Check logs above for detailed progress\n");
    }
    
    // Shutdown
    printf("\n=== Shutting Down ===\n");
    
    // Stop threads
    printf("Stopping crawler...\n");
    pthread_join(crawler_thread, NULL);
    
    printf("Stopping preprocessor...\n");
    pthread_join(preprocessor_thread, NULL);
    
    printf("Stopping tokenizer...\n");
    pthread_join(tokenizer_thread, NULL);
    
    printf("Stopping training threads...\n");
    continuous_training_stop(training, training_threads);
    
    // Cleanup
    printf("Cleaning up...\n");
    crawler_cleanup(crawler);
    preprocessor_cleanup(preprocessor);
    tokenizer_cleanup(tokenizer);
    continuous_training_cleanup(training);
    free(training_threads);
    
    printf("\n✓ Shutdown complete\n");
    
    return 0;
}