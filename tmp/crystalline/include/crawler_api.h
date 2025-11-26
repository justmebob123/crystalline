#ifndef CRAWLER_API_H
#define CRAWLER_API_H

#include <stddef.h>
#include <stdint.h>
#include "cllm.h"

/**
 * Crawler Library - High-Level API
 * 
 * This provides a clean, thread-safe API for web crawling with
 * integrated training. Use this API from both CLI tools and UI.
 */

typedef struct Crawler Crawler;

/**
 * Crawler configuration
 */
typedef struct {
    char start_url[512];
    char data_dir[512];
    int max_pages;              // 0 = unlimited
    int num_training_threads;   // Number of parallel training threads
    int min_delay_seconds;      // Minimum delay between requests (default: 2)
    int max_delay_seconds;      // Maximum delay between requests (default: 5)
    CLLMModel* model;           // Model to train (required)
} CrawlerConfig;

/**
 * Real-time crawler status
 */
typedef struct {
    int running;                // 1 if crawler is active
    int pages_crawled;          // Total pages downloaded
    int pages_preprocessed;     // Pages converted to text
    int pages_tokenized;        // Pages tokenized
    int pages_trained;          // Pages trained on
    int queue_size;             // Files waiting for training
    char current_url[512];      // URL currently being processed
    char last_error[256];       // Last error message (if any)
    double avg_loss;            // Average training loss
} CrawlerStatus;

/**
 * Create crawler instance
 * 
 * @param config Configuration (must include valid model pointer)
 * @return Crawler instance or NULL on failure
 */
Crawler* crawler_create(const CrawlerConfig* config);

/**
 * Start crawler (non-blocking - runs in background threads)
 * 
 * Starts 4 threads:
 * - Web crawler thread
 * - HTML preprocessor thread
 * - Tokenizer thread
 * - N training worker threads
 * 
 * @param crawler Crawler instance
 * @return 0 on success, -1 on failure
 */
int crawler_start(Crawler* crawler);

/**
 * Stop crawler (blocks until all threads finish cleanly)
 * 
 * @param crawler Crawler instance
 */
void crawler_stop(Crawler* crawler);

/**
 * Get current status (thread-safe, can be called from any thread)
 * 
 * @param crawler Crawler instance
 * @param status Output status structure
 */
void crawler_get_status(Crawler* crawler, CrawlerStatus* status);

/**
 * Check if crawler is running
 * 
 * @param crawler Crawler instance
 * @return 1 if running, 0 if stopped
 */
int crawler_is_running(Crawler* crawler);

/**
 * Destroy crawler instance and free all resources
 * 
 * @param crawler Crawler instance
 */
void crawler_destroy(Crawler* crawler);

/**
 * Get default configuration
 * 
 * @return Default configuration with sensible values
 */
CrawlerConfig crawler_default_config(void);

#endif // CRAWLER_API_H