/**
 * CLLM Crawler - Command Line Tool
 * 
 * Simple CLI wrapper around libcrawler.so
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "crawler_api.h"
#include "cllm.h"

static volatile int g_running = 1;
static Crawler* g_crawler = NULL;

void signal_handler(int signum) {
    (void)signum;
    printf("\n\n=== SHUTDOWN SIGNAL RECEIVED ===\n");
    g_running = 0;
}

void print_usage(const char* prog) {
    printf("Usage: %s [OPTIONS]\n", prog);
    printf("\nOptions:\n");
    printf("  --start-url URL      Starting URL for crawler (required)\n");
    printf("  --max-pages N        Maximum pages to crawl (default: 0 = unlimited)\n");
    printf("  --data-dir PATH      Data directory (default: crawler_data)\n");
    printf("  --model-path PATH    Model file path (required)\n");
    printf("  --threads N          Number of training threads (default: 1)\n");
    printf("  --help               Show this help\n");
    printf("\nExample:\n");
    printf("  %s --start-url https://example.com --model-path models/model.cllm\n", prog);
}

int main(int argc, char** argv) {
    // Parse arguments
    CrawlerConfig config = crawler_default_config();
    char model_path[512] = "";
    int has_start_url = 0;
    int has_model = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--start-url") == 0 && i + 1 < argc) {
            strncpy(config.start_url, argv[++i], sizeof(config.start_url) - 1);
            has_start_url = 1;
        } else if (strcmp(argv[i], "--max-pages") == 0 && i + 1 < argc) {
            config.max_pages = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--data-dir") == 0 && i + 1 < argc) {
            strncpy(config.data_dir, argv[++i], sizeof(config.data_dir) - 1);
        } else if (strcmp(argv[i], "--model-path") == 0 && i + 1 < argc) {
            strncpy(model_path, argv[++i], sizeof(model_path) - 1);
            has_model = 1;
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            config.num_training_threads = atoi(argv[++i]);
        }
    }
    
    if (!has_start_url || !has_model) {
        fprintf(stderr, "Error: --start-url and --model-path are required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Print configuration
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("        CLLM CRAWLER - Continuous Learning System\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("Configuration:\n");
    printf("  Start URL: %s\n", config.start_url);
    printf("  Max pages: %s\n", config.max_pages == 0 ? "UNLIMITED" : "");
    if (config.max_pages > 0) printf("  Max pages: %d\n", config.max_pages);
    printf("  Data directory: %s\n", config.data_dir);
    printf("  Model path: %s\n", model_path);
    printf("  Training threads: %d\n", config.num_training_threads);
    printf("═══════════════════════════════════════════════════════════════\n\n");
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Load model
    printf("=== Loading Model ===\n");
    extern CLLMModel* cllm_read_model(const char* filepath);
    CLLMModel* model = cllm_read_model(model_path);
    
    if (!model) {
        fprintf(stderr, "Failed to load model from: %s\n", model_path);
        return 1;
    }
    
    printf("✓ Model loaded from: %s\n", model_path);
    config.model = model;
    strncpy(config.model_path, model_path, sizeof(config.model_path) - 1);
    
    // Create crawler
    printf("\n=== Creating Crawler ===\n");
    g_crawler = crawler_create(&config);
    if (!g_crawler) {
        fprintf(stderr, "Failed to create crawler\n");
        return 1;
    }
    printf("✓ Crawler created\n");
    
    // Start crawler
    printf("\n=== Starting Crawler ===\n");
    if (crawler_start(g_crawler) != 0) {
        fprintf(stderr, "Failed to start crawler\n");
        crawler_destroy(g_crawler);
        return 1;
    }
    printf("✓ Crawler started\n");
    printf("\n=== System Running ===\n");
    printf("Press Ctrl+C to stop\n\n");
    
    // Status monitoring loop
    CrawlerStatus status;
    int last_crawled = 0, last_preprocessed = 0, last_tokenized = 0, last_trained = 0;
    
    while (g_running && crawler_is_running(g_crawler)) {
        sleep(5);
        
        crawler_get_status(g_crawler, &status);
        
        // Print status if anything changed
        if (status.pages_crawled != last_crawled ||
            status.pages_preprocessed != last_preprocessed ||
            status.pages_tokenized != last_tokenized ||
            status.pages_trained != last_trained) {
            
            printf("\n=== STATUS UPDATE ===\n");
            printf("  Pages crawled: %d\n", status.pages_crawled);
            printf("  Pages preprocessed: %d\n", status.pages_preprocessed);
            printf("  Pages tokenized: %d\n", status.pages_tokenized);
            printf("  Pages trained: %d\n", status.pages_trained);
            printf("  Training queue: %d\n", status.queue_size);
            if (status.current_url[0]) {
                printf("  Current URL: %s\n", status.current_url);
            }
            
            last_crawled = status.pages_crawled;
            last_preprocessed = status.pages_preprocessed;
            last_tokenized = status.pages_tokenized;
            last_trained = status.pages_trained;
        }
    }
    
    // Shutdown
    printf("\n\n=== Shutting Down ===\n");
    crawler_stop(g_crawler);
    crawler_destroy(g_crawler);
    
    printf("✓ Shutdown complete\n");
    return 0;
}