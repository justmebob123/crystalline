/**
 * Crawler Thread Integration for UI
 * 
 * Simple wrapper around libcrawler.so for UI integration
 */

#include "crawler_thread.h"
#include "crawler_api.h"
#include "app_common.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

// Global crawler instance
static Crawler* g_crawler = NULL;
static pthread_mutex_t g_crawler_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Start crawler thread
 */
int start_crawler_thread(AppState* state, const char* start_url) {
    pthread_mutex_lock(&g_crawler_mutex);
    
    if (g_crawler != NULL) {
        printf("Crawler is already running\n");
        pthread_mutex_unlock(&g_crawler_mutex);
        return -1;
    }
    
    // Create configuration
    CrawlerConfig config = crawler_default_config();
    strncpy(config.start_url, start_url, sizeof(config.start_url) - 1);
    strncpy(config.data_dir, state->crawler_data_dir, sizeof(config.data_dir) - 1);
    config.max_pages = 0;  // Unlimited
    config.num_training_threads = 1;
    config.model = state->cllm_model;
    
    // Get model path
    extern void workspace_get_model_path(AppState* state, char* output, size_t output_size);
    workspace_get_model_path(state, config.model_path, sizeof(config.model_path));
    
    // Create crawler
    g_crawler = crawler_create(&config);
    if (!g_crawler) {
        printf("ERROR: Failed to create crawler\n");
        pthread_mutex_unlock(&g_crawler_mutex);
        return -1;
    }
    
    // Start crawler
    if (crawler_start(g_crawler) != 0) {
        printf("ERROR: Failed to start crawler\n");
        crawler_destroy(g_crawler);
        g_crawler = NULL;
        pthread_mutex_unlock(&g_crawler_mutex);
        return -1;
    }
    
    printf("✓ Crawler started successfully\n");
    pthread_mutex_unlock(&g_crawler_mutex);
    return 0;
}

/**
 * Stop crawler thread
 */
void stop_crawler_thread(void) {
    pthread_mutex_lock(&g_crawler_mutex);
    
    if (g_crawler == NULL) {
        pthread_mutex_unlock(&g_crawler_mutex);
        return;
    }
    
    printf("Stopping crawler...\n");
    crawler_stop(g_crawler);
    crawler_destroy(g_crawler);
    g_crawler = NULL;
    
    printf("✓ Crawler stopped\n");
    pthread_mutex_unlock(&g_crawler_mutex);
}

/**
 * Check if crawler is running
 */
int is_crawler_running(void) {
    pthread_mutex_lock(&g_crawler_mutex);
    int running = (g_crawler != NULL && crawler_is_running(g_crawler));
    pthread_mutex_unlock(&g_crawler_mutex);
    return running;
}

/**
 * Get crawler status
 */
void get_crawler_status(int* pages_crawled, int* queue_size) {
    pthread_mutex_lock(&g_crawler_mutex);
    
    if (g_crawler == NULL) {
        if (pages_crawled) *pages_crawled = 0;
        if (queue_size) *queue_size = 0;
        pthread_mutex_unlock(&g_crawler_mutex);
        return;
    }
    
    CrawlerStatus status;
    crawler_get_status(g_crawler, &status);
    
    if (pages_crawled) *pages_crawled = status.pages_crawled;
    if (queue_size) *queue_size = status.queue_size;
    
    pthread_mutex_unlock(&g_crawler_mutex);
}

/**
 * Get detailed crawler status
 */
void get_crawler_status_detailed(CrawlerStatus* status) {
    pthread_mutex_lock(&g_crawler_mutex);
    
    if (g_crawler == NULL || status == NULL) {
        pthread_mutex_unlock(&g_crawler_mutex);
        return;
    }
    
    crawler_get_status(g_crawler, status);
    
    pthread_mutex_unlock(&g_crawler_mutex);
}