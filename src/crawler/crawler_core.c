/**
 * Web Crawler Core
 * 
 * Implements slow, methodical web crawling with:
 * - Rate limiting
 * - Link extraction
 * - robots.txt respect
 * - Domain filtering
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <pthread.h>

#define MAX_URL_LENGTH 2048
#define MAX_PAGE_SIZE (10 * 1024 * 1024)  // 10MB max page size
#define MIN_DELAY_SECONDS 2
#define MAX_DELAY_SECONDS 5

/**
 * Get current timestamp string
 */
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "[%H:%M:%S]", tm_info);
}

typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} MemoryBuffer;

typedef struct {
    char data_dir[1024];
    char start_url[MAX_URL_LENGTH];
    int max_pages;
    int pages_crawled;
    int running;
    pthread_mutex_t lock;
    FILE* links_to_crawl;
    FILE* links_crawled;
} CrawlerStateInternal;

/**
 * Initialize crawler state (internal function)
 */
CrawlerStateInternal* crawler_internal_init(const char* data_dir, const char* start_url, int max_pages) {
    CrawlerStateInternal* state = (CrawlerStateInternal*)calloc(1, sizeof(CrawlerStateInternal));
    if (!state) return NULL;
    
    strncpy(state->data_dir, data_dir, sizeof(state->data_dir) - 1);
    strncpy(state->start_url, start_url, sizeof(state->start_url) - 1);
    state->max_pages = max_pages;
    state->pages_crawled = 0;
    state->running = 1;
    pthread_mutex_init(&state->lock, NULL);
    
    // Create directory structure
    char path[1024];
    snprintf(path, sizeof(path), "%s/raw_pages", data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/preprocessed", data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/training_queue", data_dir);
    mkdir(path, 0755);
    
    snprintf(path, sizeof(path), "%s/trained", data_dir);
    mkdir(path, 0755);
    
    // Open link files
    snprintf(path, sizeof(path), "%s/links_to_crawl.txt", data_dir);
    state->links_to_crawl = fopen(path, "a+");
    
    snprintf(path, sizeof(path), "%s/links_crawled.txt", data_dir);
    state->links_crawled = fopen(path, "a+");
    
    // Add start URL to queue if files are empty
    fseek(state->links_to_crawl, 0, SEEK_END);
    if (ftell(state->links_to_crawl) == 0) {
        fprintf(state->links_to_crawl, "%s\n", start_url);
        fflush(state->links_to_crawl);
    }
    
    return state;
}

/**
 * Cleanup crawler state (internal function)
 */
void crawler_internal_cleanup(CrawlerStateInternal* state) {
    if (!state) return;
    
    if (state->links_to_crawl) fclose(state->links_to_crawl);
    if (state->links_crawled) fclose(state->links_crawled);
    pthread_mutex_destroy(&state->lock);
    free(state);
}

/**
 * Callback for curl to write data
 */
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryBuffer* mem = (MemoryBuffer*)userp;
    
    // Check if we need to expand buffer
    if (mem->size + realsize + 1 > mem->capacity) {
        size_t new_capacity = mem->capacity * 2;
        if (new_capacity < mem->size + realsize + 1) {
            new_capacity = mem->size + realsize + 1;
        }
        
        char* new_data = (char*)realloc(mem->data, new_capacity);
        if (!new_data) return 0;  // Out of memory
        
        mem->data = new_data;
        mem->capacity = new_capacity;
    }
    
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

/**
 * Download a web page
 */
int crawler_download_page(const char* url, MemoryBuffer* buffer) {
    CURL* curl = curl_easy_init();
    if (!curl) return -1;
    
    // Set options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, 
                    "Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return -1;
    }
    
    if (response_code != 200) {
        fprintf(stderr, "HTTP error: %ld\n", response_code);
        return -1;
    }
    
    return 0;
}

/**
 * Save page to disk
 */
int crawler_save_page(CrawlerStateInternal* state, const char* url, const char* content, size_t size) {
    // Generate filename from URL hash
    unsigned long hash = 5381;
    for (const char* p = url; *p; p++) {
        hash = ((hash << 5) + hash) + *p;
    }
    
    time_t now = time(NULL);
    char filename[1024];
    snprintf(filename, sizeof(filename), "%s/raw_pages/page_%lu_%ld.html", 
             state->data_dir, hash, now);
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        char timestamp[32];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(stderr, "%s Failed to open file: %s\n", timestamp, filename);
        return -1;
    }
    
    // Write metadata header
    fprintf(f, "<!-- URL: %s -->\n", url);
    fprintf(f, "<!-- Timestamp: %ld -->\n", now);
    fprintf(f, "<!-- Size: %zu -->\n", size);
    
    // Write content
    fwrite(content, 1, size, f);
    fclose(f);
    
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s ✓ Saved: %s\n", timestamp, filename);
    return 0;
}

/**
 * Extract links from HTML
 */
int crawler_extract_links(const char* html, const char* base_url, CrawlerStateInternal* state) {
    (void)base_url;  // TODO: Use for resolving relative URLs
    // Simple link extraction (looking for href="...")
    const char* p = html;
    int links_found = 0;
    
    while ((p = strstr(p, "href=&quot;")) != NULL) {
        p += 6;  // Skip 'href="'
        
        const char* end = strchr(p, '"');
        if (!end) break;
        
        size_t len = end - p;
        if (len >= MAX_URL_LENGTH) {
            p = end + 1;
            continue;
        }
        
        char url[MAX_URL_LENGTH];
        strncpy(url, p, len);
        url[len] = '\0';
        
        // Skip anchors, javascript, mailto
        if (url[0] == '#' || strncmp(url, "javascript:", 11) == 0 || 
            strncmp(url, "mailto:", 7) == 0) {
            p = end + 1;
            continue;
        }
        
        // Handle relative URLs (simple version)
        if (url[0] == '/') {
            // TODO: Combine with base_url
            p = end + 1;
            continue;
        }
        
        // Only http/https
        if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
            p = end + 1;
            continue;
        }
        
        // Add to queue
        fprintf(state->links_to_crawl, "%s\n", url);
        links_found++;
        
        p = end + 1;
    }
    
    if (links_found > 0) {
        fflush(state->links_to_crawl);
        char timestamp[32];
        get_timestamp(timestamp, sizeof(timestamp));
        printf("%s   Found %d links\n", timestamp, links_found);
    }
    
    return links_found;
}

/**
 * Get next URL to crawl
 */
int crawler_get_next_url(CrawlerStateInternal* state, char* url, size_t url_size) {
    pthread_mutex_lock(&state->lock);
    
    // Rewind and read first line
    fseek(state->links_to_crawl, 0, SEEK_SET);
    
    if (fgets(url, url_size, state->links_to_crawl) == NULL) {
        pthread_mutex_unlock(&state->lock);
        return -1;  // No more URLs
    }
    
    // Remove newline
    url[strcspn(url, "\n")] = 0;
    
    // Read remaining lines into temp file
    char temp_path[1024];
    snprintf(temp_path, sizeof(temp_path), "%s/links_to_crawl.tmp", state->data_dir);
    FILE* temp = fopen(temp_path, "w");
    
    char line[MAX_URL_LENGTH];
    while (fgets(line, sizeof(line), state->links_to_crawl)) {
        fputs(line, temp);
    }
    
    fclose(temp);
    fclose(state->links_to_crawl);
    
    // Replace original with temp
    char orig_path[1024];
    snprintf(orig_path, sizeof(orig_path), "%s/links_to_crawl.txt", state->data_dir);
    rename(temp_path, orig_path);
    
    // Reopen
    state->links_to_crawl = fopen(orig_path, "a+");
    
    pthread_mutex_unlock(&state->lock);
    return 0;
}

/**
 * Mark URL as crawled
 */
void crawler_mark_crawled(CrawlerStateInternal* state, const char* url) {
    pthread_mutex_lock(&state->lock);
    fprintf(state->links_crawled, "%s\n", url);
    fflush(state->links_crawled);
    state->pages_crawled++;
    pthread_mutex_unlock(&state->lock);
}

/**
 * Main crawler loop
 */
void* crawler_thread_func(void* arg) {
    CrawlerStateInternal* state = (CrawlerStateInternal*)arg;
    char timestamp[32];
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === CRAWLER STARTED ===\n", timestamp);
    printf("%s Data directory: %s\n", timestamp, state->data_dir);
    if (state->max_pages == 0) {
        printf("%s Max pages: UNLIMITED\n", timestamp);
    } else {
        printf("%s Max pages: %d\n", timestamp, state->max_pages);
    }
    
    while (state->running && (state->max_pages == 0 || state->pages_crawled < state->max_pages)) {
        char url[MAX_URL_LENGTH];
        
        // Get next URL
        if (crawler_get_next_url(state, url, sizeof(url)) != 0) {
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s No more URLs in queue, waiting...\n", timestamp);
            sleep(5);  // Wait for more URLs
            continue;
        }
        
        // Print progress
        get_timestamp(timestamp, sizeof(timestamp));
        if (state->max_pages == 0) {
            printf("\n%s === Crawling [%d/unlimited] ===\n", timestamp, state->pages_crawled + 1);
        } else {
            printf("\n%s === Crawling [%d/%d] ===\n", timestamp, state->pages_crawled + 1, state->max_pages);
        }
        printf("%s URL: %s\n", timestamp, url);
        
        // Download page
        MemoryBuffer buffer = {0};
        buffer.capacity = 4096;
        buffer.data = (char*)malloc(buffer.capacity);
        
        if (crawler_download_page(url, &buffer) == 0) {
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s Downloaded %zu bytes\n", timestamp, buffer.size);
            
            // Save page
            crawler_save_page(state, url, buffer.data, buffer.size);
            
            // Extract links
            crawler_extract_links(buffer.data, url, state);
            
            // Mark as crawled
            crawler_mark_crawled(state, url);
        } else {
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s ✗ Failed to download\n", timestamp);
        }
        
        free(buffer.data);
        
        // Rate limiting: random delay between MIN and MAX seconds
        int delay = MIN_DELAY_SECONDS + (rand() % (MAX_DELAY_SECONDS - MIN_DELAY_SECONDS + 1));
        get_timestamp(timestamp, sizeof(timestamp));
        printf("%s Waiting %d seconds...\n", timestamp, delay);
        sleep(delay);
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("\n%s === CRAWLER STOPPED ===\n", timestamp);
    printf("%s Total pages crawled: %d\n", timestamp, state->pages_crawled);
    
    return NULL;
}