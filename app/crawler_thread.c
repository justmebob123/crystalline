// For popen/pclose
#define _POSIX_C_SOURCE 200809L

#include "crawler_thread.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

static pthread_t crawler_thread_id = 0;
static int crawler_running = 0;
static int crawler_pages_crawled = 0;
static int crawler_queue_size = 0;
static pthread_mutex_t crawler_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char start_url[512];
    char data_dir[512];
    int max_pages;
} CrawlerThreadArgs;

static void* crawler_thread_func(void* arg) {
    CrawlerThreadArgs* args = (CrawlerThreadArgs*)arg;
    
    // Get timestamp
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
    
    printf("\n[%s] === CRAWLER THREAD STARTED ===\n", timestamp);
    printf("Start URL: %s\n", args->start_url);
    printf("Data directory: %s\n", args->data_dir);
    if (args->max_pages == 0) {
        printf("Max pages: UNLIMITED\n");
    } else {
        printf("Max pages: %d\n", args->max_pages);
    }
    
    // Create data directories
    printf("Creating directory structure...\n");
    printf("  Main directory: %s\n", args->data_dir);
    fflush(stdout);
    
    int dir_result = mkdir(args->data_dir, 0755);  // Create main data directory first!
    if (dir_result != 0 && errno != EEXIST) {
        printf("  ERROR: Failed to create %s (errno=%d)\n", args->data_dir, errno);
        fflush(stdout);
    }
    
    char raw_dir[1024], preprocessed_dir[1024], training_queue_dir[1024], trained_dir[1024];
    snprintf(raw_dir, sizeof(raw_dir), "%s/raw_pages", args->data_dir);
    snprintf(preprocessed_dir, sizeof(preprocessed_dir), "%s/preprocessed", args->data_dir);
    snprintf(training_queue_dir, sizeof(training_queue_dir), "%s/training_queue", args->data_dir);
    snprintf(trained_dir, sizeof(trained_dir), "%s/trained", args->data_dir);
    
    mkdir(raw_dir, 0755);
    mkdir(preprocessed_dir, 0755);
    mkdir(training_queue_dir, 0755);
    mkdir(trained_dir, 0755);
    
    printf("✓ Directories created\n");
    
    // Build command to run cllm_crawler with proper library path
    // Use the same model as the main app
    char command[2048];
    snprintf(command, sizeof(command),
             "cd %s && LD_LIBRARY_PATH=.. ../tools/cllm_crawler --start-url '%s' --max-pages %d --data-dir '%s' --model-path '../models/saved_model.cllm' > crawler.log 2>&1 &",
             args->data_dir, args->start_url, args->max_pages, args->data_dir);
    
    printf("Executing: %s\n", command);
    fflush(stdout);
    
    int result = system(command);
    printf("Command returned: %d\n", result);
    fflush(stdout);
    
    if (result == 0) {
        printf("Crawler process started successfully\n");
        fflush(stdout);
        
        // Monitor crawler progress
        char log_file[1024];
        snprintf(log_file, sizeof(log_file), "%s/crawler.log", args->data_dir);
        
        printf("\n=== CRAWLER MONITORING STARTED ===\n");
        printf("Watching: %s\n", log_file);
        printf("Raw pages: %s\n", raw_dir);
        printf("Preprocessed: %s\n", preprocessed_dir);
        printf("Training queue: %s\n", training_queue_dir);
        printf("Trained: %s\n\n", trained_dir);
        
        int last_raw = 0, last_preprocessed = 0, last_queue = 0, last_trained = 0;
        
        while (crawler_running) {
            // Update status by checking directories
            pthread_mutex_lock(&crawler_mutex);
            
            // Count files in each directory
            char count_cmd[1024];
            int raw_count = 0, preprocessed_count = 0, queue_count = 0, trained_count = 0;
            
            snprintf(count_cmd, sizeof(count_cmd), "ls -1 %.900s 2>/dev/null | wc -l", raw_dir);
            FILE* fp = popen(count_cmd, "r");
            if (fp) {
                if (fscanf(fp, "%d", &raw_count) == 1) {
                    crawler_pages_crawled = raw_count;
                }
                pclose(fp);
            }
            
            snprintf(count_cmd, sizeof(count_cmd), "ls -1 %.900s 2>/dev/null | wc -l", preprocessed_dir);
            fp = popen(count_cmd, "r");
            if (fp) {
                fscanf(fp, "%d", &preprocessed_count);
                pclose(fp);
            }
            
            snprintf(count_cmd, sizeof(count_cmd), "ls -1 %.900s 2>/dev/null | wc -l", training_queue_dir);
            fp = popen(count_cmd, "r");
            if (fp) {
                if (fscanf(fp, "%d", &queue_count) == 1) {
                    crawler_queue_size = queue_count;
                }
                pclose(fp);
            }
            
            snprintf(count_cmd, sizeof(count_cmd), "ls -1 %.900s 2>/dev/null | wc -l", trained_dir);
            fp = popen(count_cmd, "r");
            if (fp) {
                fscanf(fp, "%d", &trained_count);
                pclose(fp);
            }
            
            pthread_mutex_unlock(&crawler_mutex);
            
            // Print status if anything changed
            if (raw_count != last_raw || preprocessed_count != last_preprocessed || 
                queue_count != last_queue || trained_count != last_trained) {
                // Get timestamp
                time_t now = time(NULL);
                struct tm* tm_info = localtime(&now);
                char timestamp[32];
                strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
                
                printf("[%s] CRAWLER STATUS: Raw: %d | Preprocessed: %d | Queue: %d | Trained: %d\n",
                       timestamp, raw_count, preprocessed_count, queue_count, trained_count);
                
                if (raw_count > last_raw) {
                    printf("  → Downloaded %d new page(s)\n", raw_count - last_raw);
                }
                if (preprocessed_count > last_preprocessed) {
                    printf("  → Preprocessed %d new page(s)\n", preprocessed_count - last_preprocessed);
                }
                if (queue_count > last_queue) {
                    printf("  → Tokenized %d new page(s)\n", queue_count - last_queue);
                }
                if (trained_count > last_trained) {
                    printf("  → Trained on %d new page(s)\n", trained_count - last_trained);
                }
                
                last_raw = raw_count;
                last_preprocessed = preprocessed_count;
                last_queue = queue_count;
                last_trained = trained_count;
            }
            
            // Sleep for 5 seconds before next update
            sleep(5);
        }
        
        printf("\n=== CRAWLER MONITORING STOPPED ===\n");
    } else {
        printf("ERROR: Failed to start crawler process (exit code: %d)\n", result);
    }
    
    free(args);
    printf("=== CRAWLER THREAD STOPPED ===\n");
    return NULL;
}

int start_crawler_thread(AppState* state, const char* start_url) {
    if (crawler_running) {
        printf("Crawler is already running\n");
        return -1;
    }
    
    // Allocate args
    CrawlerThreadArgs* args = (CrawlerThreadArgs*)malloc(sizeof(CrawlerThreadArgs));
    if (!args) {
        printf("ERROR: Failed to allocate crawler thread args\n");
        return -1;
    }
    
    // Copy parameters
    strncpy(args->start_url, start_url, sizeof(args->start_url) - 1);
    args->start_url[sizeof(args->start_url) - 1] = '\0';
    
    strncpy(args->data_dir, state->crawler_data_dir, sizeof(args->data_dir) - 1);
    args->data_dir[sizeof(args->data_dir) - 1] = '\0';
    
    args->max_pages = 0; // 0 = unlimited
    
    // Set running flag
    crawler_running = 1;
    crawler_pages_crawled = 0;
    crawler_queue_size = 0;
    
    // Create thread
    if (pthread_create(&crawler_thread_id, NULL, crawler_thread_func, args) != 0) {
        printf("ERROR: Failed to create crawler thread\n");
        free(args);
        crawler_running = 0;
        return -1;
    }
    
    printf("Crawler thread created successfully\n");
    return 0;
}

void stop_crawler_thread(void) {
    if (!crawler_running) {
        return;
    }
    
    printf("Stopping crawler thread...\n");
    crawler_running = 0;
    
    if (crawler_thread_id) {
        pthread_join(crawler_thread_id, NULL);
        crawler_thread_id = 0;
    }
    
    printf("Crawler thread stopped\n");
}

int is_crawler_running(void) {
    return crawler_running;
}

void get_crawler_status(int* pages_crawled, int* queue_size) {
    pthread_mutex_lock(&crawler_mutex);
    if (pages_crawled) *pages_crawled = crawler_pages_crawled;
    if (queue_size) *queue_size = crawler_queue_size;
    pthread_mutex_unlock(&crawler_mutex);
}