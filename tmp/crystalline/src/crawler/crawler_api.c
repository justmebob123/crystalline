/**
 * Crawler Library - High-Level API Implementation
 * 
 * Wraps the low-level crawler components into a clean library API
 */

#include "crawler_api.h"
#include "crawler.h"
#include "cllm_training.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

struct Crawler {
    // Configuration
    CrawlerConfig config;
    
    // Component states
    CrawlerState* crawler_state;
    PreprocessorState* preprocessor_state;
    TokenizerState* tokenizer_state;
    ContinuousTrainingState* training_state;
    
    // Threads
    pthread_t crawler_thread;
    pthread_t preprocessor_thread;
    pthread_t tokenizer_thread;
    pthread_t* training_threads;
    
    // Status tracking
    CrawlerStatus status;
    pthread_mutex_t status_mutex;
    
    // Running flag
    int running;
};

/**
 * Get default configuration
 */
CrawlerConfig crawler_default_config(void) {
    CrawlerConfig config = {0};
    config.max_pages = 0;  // Unlimited
    config.num_training_threads = 1;
    config.min_delay_seconds = 2;
    config.max_delay_seconds = 5;
    strcpy(config.data_dir, "crawler_data");
    return config;
}

/**
 * Create crawler instance
 */
Crawler* crawler_create(const CrawlerConfig* config) {
    if (!config || !config->model) {
        return NULL;
    }
    
    Crawler* crawler = (Crawler*)calloc(1, sizeof(Crawler));
    if (!crawler) return NULL;
    
    // Copy configuration
    memcpy(&crawler->config, config, sizeof(CrawlerConfig));
    
    // Initialize mutex
    pthread_mutex_init(&crawler->status_mutex, NULL);
    
    // Create data directories
    mkdir(config->data_dir, 0755);
    
    char path[1024];
    snprintf(path, sizeof(path), "%s/raw_pages", config->data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/preprocessed", config->data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/training_queue", config->data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/trained", config->data_dir);
    mkdir(path, 0755);
    
    // Initialize status
    crawler->status.running = 0;
    
    return crawler;
}

/**
 * Start crawler
 */
int crawler_start(Crawler* crawler) {
    if (!crawler || crawler->running) {
        return -1;
    }
    
    // Initialize components
    crawler->crawler_state = crawler_init(
        crawler->config.data_dir,
        crawler->config.start_url,
        crawler->config.max_pages
    );
    
    crawler->preprocessor_state = preprocessor_init(crawler->config.data_dir);
    crawler->tokenizer_state = tokenizer_init(crawler->config.data_dir);
    
    crawler->training_state = continuous_training_init(
        crawler->config.data_dir,
        crawler->config.model_path,
        crawler->config.model,
        crawler->config.num_training_threads
    );
    
    if (!crawler->crawler_state || !crawler->preprocessor_state || 
        !crawler->tokenizer_state || !crawler->training_state) {
        return -1;
    }
    
    // Allocate training threads array
    crawler->training_threads = (pthread_t*)malloc(
        crawler->config.num_training_threads * sizeof(pthread_t)
    );
    
    // Start threads
    if (pthread_create(&crawler->crawler_thread, NULL, 
                      crawler_thread_func, crawler->crawler_state) != 0) {
        return -1;
    }
    
    if (pthread_create(&crawler->preprocessor_thread, NULL,
                      preprocessor_thread_func, crawler->preprocessor_state) != 0) {
        return -1;
    }
    
    if (pthread_create(&crawler->tokenizer_thread, NULL,
                      tokenizer_thread_func, crawler->tokenizer_state) != 0) {
        return -1;
    }
    
    if (continuous_training_start(crawler->training_state, 
                                  crawler->training_threads) != 0) {
        return -1;
    }
    
    // Mark as running
    crawler->running = 1;
    pthread_mutex_lock(&crawler->status_mutex);
    crawler->status.running = 1;
    pthread_mutex_unlock(&crawler->status_mutex);
    
    return 0;
}

/**
 * Stop crawler
 */
void crawler_stop(Crawler* crawler) {
    if (!crawler || !crawler->running) {
        return;
    }
    
    // Mark as stopping
    crawler->running = 0;
    
    // Stop components (they check their running flags)
    if (crawler->crawler_state) {
        crawler->crawler_state->running = 0;
    }
    if (crawler->preprocessor_state) {
        crawler->preprocessor_state->running = 0;
    }
    if (crawler->tokenizer_state) {
        crawler->tokenizer_state->running = 0;
    }
    
    // Wait for threads to finish
    pthread_join(crawler->crawler_thread, NULL);
    pthread_join(crawler->preprocessor_thread, NULL);
    pthread_join(crawler->tokenizer_thread, NULL);
    
    if (crawler->training_state) {
        continuous_training_stop(crawler->training_state, crawler->training_threads);
    }
    
    // Update status
    pthread_mutex_lock(&crawler->status_mutex);
    crawler->status.running = 0;
    pthread_mutex_unlock(&crawler->status_mutex);
}

/**
 * Get current status
 */
void crawler_get_status(Crawler* crawler, CrawlerStatus* status) {
    if (!crawler || !status) return;
    
    pthread_mutex_lock(&crawler->status_mutex);
    
    // Update counts by checking directories
    char path[1024];
    DIR* dir;
    struct dirent* entry;
    
    // Count raw pages
    snprintf(path, sizeof(path), "%s/raw_pages", crawler->config.data_dir);
    dir = opendir(path);
    if (dir) {
        int count = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') count++;
        }
        closedir(dir);
        crawler->status.pages_crawled = count;
    }
    
    // Count preprocessed
    snprintf(path, sizeof(path), "%s/preprocessed", crawler->config.data_dir);
    dir = opendir(path);
    if (dir) {
        int count = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') count++;
        }
        closedir(dir);
        crawler->status.pages_preprocessed = count;
    }
    
    // Count tokenized (queue)
    snprintf(path, sizeof(path), "%s/training_queue", crawler->config.data_dir);
    dir = opendir(path);
    if (dir) {
        int count = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.' && strstr(entry->d_name, ".tok")) {
                count++;
            }
        }
        closedir(dir);
        crawler->status.queue_size = count;
        crawler->status.pages_tokenized = count;
    }
    
    // Count trained
    snprintf(path, sizeof(path), "%s/trained", crawler->config.data_dir);
    dir = opendir(path);
    if (dir) {
        int count = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') count++;
        }
        closedir(dir);
        crawler->status.pages_trained = count;
    }
    
    // Copy status
    memcpy(status, &crawler->status, sizeof(CrawlerStatus));
    
    pthread_mutex_unlock(&crawler->status_mutex);
}

/**
 * Check if running
 */
int crawler_is_running(Crawler* crawler) {
    if (!crawler) return 0;
    return crawler->running;
}

/**
 * Destroy crawler
 */
void crawler_destroy(Crawler* crawler) {
    if (!crawler) return;
    
    // Stop if running
    if (crawler->running) {
        crawler_stop(crawler);
    }
    
    // Cleanup components
    if (crawler->crawler_state) {
        crawler_cleanup(crawler->crawler_state);
    }
    if (crawler->preprocessor_state) {
        preprocessor_cleanup(crawler->preprocessor_state);
    }
    if (crawler->tokenizer_state) {
        tokenizer_cleanup(crawler->tokenizer_state);
    }
    if (crawler->training_state) {
        continuous_training_cleanup(crawler->training_state);
    }
    
    // Free training threads array
    if (crawler->training_threads) {
        free(crawler->training_threads);
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&crawler->status_mutex);
    
    // Free crawler
    free(crawler);
}

#endif // CRAWLER_API_H