#ifndef CRAWLER_H
#define CRAWLER_H

#include <stddef.h>
#include <stdint.h>

/**
 * Crawler Library - Core Web Crawling Functionality
 * 
 * This library provides thread-safe web crawling with:
 * - Rate-limited page downloading
 * - HTML preprocessing
 * - Text tokenization
 * - Continuous training integration
 */

// Forward declarations
typedef struct Crawler Crawler;
typedef struct CrawlerConfig CrawlerConfig;
typedef struct CrawlerStatus CrawlerStatus;

/**
 * Crawler configuration
 */
struct CrawlerConfig {
    char start_url[512];
    char data_dir[512];
    char model_path[512];
    int max_pages;              // 0 = unlimited
    int num_training_threads;   // Number of parallel training threads
    int min_delay_seconds;      // Minimum delay between requests
    int max_delay_seconds;      // Maximum delay between requests
};

/**
 * Real-time crawler status
 */
struct CrawlerStatus {
    int running;                // 1 if crawler is active
    int pages_crawled;          // Total pages downloaded
    int pages_preprocessed;     // Pages converted to text
    int pages_tokenized;        // Pages tokenized
    int pages_trained;          // Pages trained on
    int queue_size;             // Files waiting for training
    char current_url[512];      // URL currently being processed
    char last_error[256];       // Last error message (if any)
};

/**
 * Create crawler instance
 */
Crawler* crawler_create(const CrawlerConfig* config);

/**
 * Start crawler (non-blocking - runs in background threads)
 */
int crawler_start(Crawler* crawler);

/**
 * Stop crawler (blocks until all threads finish)
 */
void crawler_stop(Crawler* crawler);

/**
 * Get current status (thread-safe)
 */
void crawler_get_status(Crawler* crawler, CrawlerStatus* status);

/**
 * Check if crawler is running
 */
int crawler_is_running(Crawler* crawler);

/**
 * Destroy crawler instance
 */
void crawler_destroy(Crawler* crawler);

/**
 * Get default configuration
 */
CrawlerConfig crawler_default_config(void);

#endif // CRAWLER_H