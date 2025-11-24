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
    
    printf("=== CRAWLER THREAD STARTED ===\n");
    printf("Start URL: %s\n", args->start_url);
    printf("Data directory: %s\n", args->data_dir);
    printf("Max pages: %d\n", args->max_pages);
    
    // Create data directories
    char raw_dir[1024], preprocessed_dir[1024], training_queue_dir[1024], trained_dir[1024];
    snprintf(raw_dir, sizeof(raw_dir), "%s/raw_pages", args->data_dir);
    snprintf(preprocessed_dir, sizeof(preprocessed_dir), "%s/preprocessed", args->data_dir);
    snprintf(training_queue_dir, sizeof(training_queue_dir), "%s/training_queue", args->data_dir);
    snprintf(trained_dir, sizeof(trained_dir), "%s/trained", args->data_dir);
    
    mkdir(raw_dir, 0755);
    mkdir(preprocessed_dir, 0755);
    mkdir(training_queue_dir, 0755);
    mkdir(trained_dir, 0755);
    
    // Build command to run cllm_crawler
    char command[2048];
    snprintf(command, sizeof(command),
             "cd %s && ../tools/cllm_crawler --start-url '%s' --max-pages %d --data-dir '%s' > crawler.log 2>&1 &",
             args->data_dir, args->start_url, args->max_pages, args->data_dir);
    
    printf("Executing: %s\n", command);
    int result = system(command);
    
    if (result == 0) {
        printf("Crawler process started successfully\n");
        
        // Monitor crawler progress
        char log_file[1024];
        snprintf(log_file, sizeof(log_file), "%s/crawler.log", args->data_dir);
        
        while (crawler_running) {
            // Update status by checking directories
            pthread_mutex_lock(&crawler_mutex);
            
            // Count files in raw_pages directory
            char count_cmd[1024];
            snprintf(count_cmd, sizeof(count_cmd), "ls -1 %.900s 2>/dev/null | wc -l", raw_dir);
            FILE* fp = popen(count_cmd, "r");
            if (fp) {
                if (fscanf(fp, "%d", &crawler_pages_crawled) != 1) {
                    crawler_pages_crawled = 0;
                }
                pclose(fp);
            }
            
            // Count files in training_queue directory
            snprintf(count_cmd, sizeof(count_cmd), "ls -1 %.900s 2>/dev/null | wc -l", training_queue_dir);
            fp = popen(count_cmd, "r");
            if (fp) {
                if (fscanf(fp, "%d", &crawler_queue_size) != 1) {
                    crawler_queue_size = 0;
                }
                pclose(fp);
            }
            
            pthread_mutex_unlock(&crawler_mutex);
            
            // Sleep for 5 seconds before next update
            sleep(5);
        }
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
    
    args->max_pages = 100; // Default max pages
    
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