#ifndef CRAWLER_H
#define CRAWLER_H

#include <pthread.h>

// Forward declarations for internal component states
typedef struct CrawlerStateInternal CrawlerStateInternal;
typedef struct PreprocessorState PreprocessorState;
typedef struct TokenizerState TokenizerState;
typedef struct ContinuousTrainingState ContinuousTrainingState;

// High-level API state (opaque to users)
typedef struct CrawlerState CrawlerState;

// ============================================================================
// STATUS AND EVENTS
// ============================================================================

/**
 * Crawler status structure (thread-safe access via crawler_get_status)
 */
typedef struct {
    int running;                    // 1 if crawler is running, 0 otherwise
    int pages_crawled;              // Number of pages downloaded
    int pages_preprocessed;         // Number of pages preprocessed
    int pages_tokenized;            // Number of pages tokenized
    int pages_trained;              // Number of pages trained on
    char current_url[512];          // Current URL being processed
    char last_error[512];           // Last error message (if any)
} CrawlerStatus;

/**
 * Event types for callbacks
 */
typedef enum {
    CRAWLER_EVENT_PAGE_DOWNLOADED,
    CRAWLER_EVENT_PAGE_PREPROCESSED,
    CRAWLER_EVENT_PAGE_TOKENIZED,
    CRAWLER_EVENT_PAGE_TRAINED,
    CRAWLER_EVENT_ERROR,
    CRAWLER_EVENT_STOPPED
} CrawlerEventType;

/**
 * Event structure passed to callbacks
 */
typedef struct {
    CrawlerEventType type;
    char message[512];
    int pages_crawled;
} CrawlerEvent;

/**
 * Callback function type
 */
typedef void (*CrawlerCallback)(const CrawlerEvent* event, void* user_data);

// ============================================================================
// MAIN CRAWLER API
// ============================================================================

/**
 * Initialize crawler state
 * @param data_dir Directory for storing crawler data
 * @param start_url Starting URL for crawling
 * @param max_pages Maximum pages to crawl (0 = unlimited)
 * @return Crawler state or NULL on error
 */
CrawlerState* crawler_state_init(const char* data_dir, const char* start_url, int max_pages);

/**
 * Start the crawler (spawns all threads internally)
 * @param state Crawler state
 * @return 0 on success, -1 on error
 */
int crawler_start(CrawlerState* state);

/**
 * Stop the crawler (stops all threads)
 * @param state Crawler state
 */
void crawler_stop(CrawlerState* state);

/**
 * Get current crawler status (thread-safe)
 * @param state Crawler state
 * @param status Output status structure
 */
void crawler_get_status(CrawlerState* state, CrawlerStatus* status);

/**
 * Set callback for crawler events
 * @param state Crawler state
 * @param callback Callback function
 * @param user_data User data passed to callback
 */
void crawler_set_callback(CrawlerState* state, CrawlerCallback callback, void* user_data);

/**
 * Cleanup crawler state
 * @param state Crawler state
 */
void crawler_state_cleanup(CrawlerState* state);

// ============================================================================
// INTERNAL FUNCTIONS (for library use only - not for application use)
// ============================================================================

// Internal component initialization
CrawlerStateInternal* crawler_internal_init(const char* data_dir, const char* start_url, int max_pages);
void crawler_internal_cleanup(CrawlerStateInternal* state);
void* crawler_thread_func(void* arg);

PreprocessorState* preprocessor_init(const char* data_dir);
void preprocessor_cleanup(PreprocessorState* state);
void* preprocessor_thread_func(void* arg);

TokenizerState* tokenizer_init(const char* data_dir);
void tokenizer_cleanup(TokenizerState* state);
void* tokenizer_thread_func(void* arg);

ContinuousTrainingState* continuous_training_init(const char* data_dir, const char* model_path, 
                                                   void* model, int num_threads);
int continuous_training_start(ContinuousTrainingState* state, pthread_t* threads);
void continuous_training_stop(ContinuousTrainingState* state, pthread_t* threads);
void continuous_training_cleanup(ContinuousTrainingState* state);

#endif // CRAWLER_H