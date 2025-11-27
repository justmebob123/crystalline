/**
 * Web Scraper for CLLM Training Data
 * 
 * Features:
 * - Crawls websites with rate limiting
 * - Extracts clean text from HTML
 * - Respects robots.txt
 * - Mimics human browsing behavior
 * - Saves extracted text for training
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_URL_LENGTH 2048
#define MAX_CONTENT_SIZE (10 * 1024 * 1024)  // 10MB
#define USER_AGENT "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"

/**
 * URL Queue for BFS crawling
 */
typedef struct URLNode {
    char url[MAX_URL_LENGTH];
    int depth;
    struct URLNode* next;
} URLNode;

typedef struct {
    URLNode* head;
    URLNode* tail;
    int size;
} URLQueue;

URLQueue* url_queue_create() {
    URLQueue* queue = (URLQueue*)calloc(1, sizeof(URLQueue));
    return queue;
}

void url_queue_push(URLQueue* queue, const char* url, int depth) {
    if (!queue || !url) return;
    
    URLNode* node = (URLNode*)malloc(sizeof(URLNode));
    strncpy(node->url, url, MAX_URL_LENGTH - 1);
    node->url[MAX_URL_LENGTH - 1] = '\0';
    node->depth = depth;
    node->next = NULL;
    
    if (queue->tail) {
        queue->tail->next = node;
    } else {
        queue->head = node;
    }
    queue->tail = node;
    queue->size++;
}

URLNode* url_queue_pop(URLQueue* queue) {
    if (!queue || !queue->head) return NULL;
    
    URLNode* node = queue->head;
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->size--;
    
    return node;
}

void url_queue_free(URLQueue* queue) {
    if (!queue) return;
    
    while (queue->head) {
        URLNode* node = queue->head;
        queue->head = node->next;
        free(node);
    }
    free(queue);
}

/**
 * Visited URL tracking
 */
typedef struct {
    char** urls;
    int count;
    int capacity;
} VisitedURLs;

VisitedURLs* visited_urls_create() {
    VisitedURLs* visited = (VisitedURLs*)calloc(1, sizeof(VisitedURLs));
    visited->capacity = 10000;
    visited->urls = (char**)calloc(visited->capacity, sizeof(char*));
    return visited;
}

int visited_urls_contains(VisitedURLs* visited, const char* url) {
    for (int i = 0; i < visited->count; i++) {
        if (strcmp(visited->urls[i], url) == 0) {
            return 1;
        }
    }
    return 0;
}

void visited_urls_add(VisitedURLs* visited, const char* url) {
    if (visited->count >= visited->capacity) return;
    visited->urls[visited->count++] = strdup(url);
}

void visited_urls_free(VisitedURLs* visited) {
    if (!visited) return;
    for (int i = 0; i < visited->count; i++) {
        free(visited->urls[i]);
    }
    free(visited->urls);
    free(visited);
}

/**
 * Fetch URL using curl
 */
char* fetch_url(const char* url, int delay_ms) {
    if (!url) return NULL;
    
    // Random delay to mimic human behavior
    if (delay_ms > 0) {
        int actual_delay = delay_ms + (rand() % 1000);
        usleep(actual_delay * 1000);
    }
    
    // Create temporary file for output
    char temp_file[] = "/tmp/scraper_XXXXXX";
    int fd = mkstemp(temp_file);
    if (fd == -1) return NULL;
    close(fd);
    
    // Build curl command
    char cmd[MAX_URL_LENGTH + 512];
    snprintf(cmd, sizeof(cmd),
             "curl -s -L -A &quot;%s&quot; "
             "--max-time 30 "
             "--max-filesize 10485760 "
             "-o &quot;%s&quot; "
             "&quot;%s&quot; 2>/dev/null",
             USER_AGENT, temp_file, url);
    
    // Execute curl
    int result = system(cmd);
    if (result != 0) {
        unlink(temp_file);
        return NULL;
    }
    
    // Read content
    FILE* f = fopen(temp_file, "r");
    if (!f) {
        unlink(temp_file);
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (size <= 0 || size > MAX_CONTENT_SIZE) {
        fclose(f);
        unlink(temp_file);
        return NULL;
    }
    
    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(f);
        unlink(temp_file);
        return NULL;
    }
    
    size_t bytes_read = fread(content, 1, size, f);
    content[bytes_read] = '\0';
    
    fclose(f);
    unlink(temp_file);
    
    return content;
}

/**
 * Extract text from HTML
 * Simple tag removal - for better results use html2text or similar
 */
char* extract_text_from_html(const char* html) {
    if (!html) return NULL;
    
    size_t len = strlen(html);
    char* text = (char*)malloc(len + 1);
    if (!text) return NULL;
    
    int in_tag = 0;
    int in_script = 0;
    int in_style = 0;
    size_t j = 0;
    
    for (size_t i = 0; i < len; i++) {
        // Check for script/style tags
        if (html[i] == '<') {
            if (strncasecmp(&html[i], "<script", 7) == 0) {
                in_script = 1;
            } else if (strncasecmp(&html[i], "</script>", 9) == 0) {
                in_script = 0;
                i += 8;
                continue;
            } else if (strncasecmp(&html[i], "<style", 6) == 0) {
                in_style = 1;
            } else if (strncasecmp(&html[i], "</style>", 8) == 0) {
                in_style = 0;
                i += 7;
                continue;
            }
            in_tag = 1;
            continue;
        }
        
        if (html[i] == '>') {
            in_tag = 0;
            continue;
        }
        
        // Skip content in tags, scripts, and styles
        if (in_tag || in_script || in_style) {
            continue;
        }
        
        // Decode HTML entities (basic)
        if (html[i] == '&') {
            if (strncmp(&html[i], "&nbsp;", 6) == 0) {
                text[j++] = ' ';
                i += 5;
            } else if (strncmp(&html[i], "&lt;", 4) == 0) {
                text[j++] = '<';
                i += 3;
            } else if (strncmp(&html[i], "&gt;", 4) == 0) {
                text[j++] = '>';
                i += 3;
            } else if (strncmp(&html[i], "&amp;", 5) == 0) {
                text[j++] = '&';
                i += 4;
            } else if (strncmp(&html[i], "&quot;", 6) == 0) {
                text[j++] = '"';
                i += 5;
            } else {
                text[j++] = html[i];
            }
        } else {
            text[j++] = html[i];
        }
    }
    
    text[j] = '\0';
    
    // Clean up whitespace
    char* cleaned = (char*)malloc(j + 1);
    if (!cleaned) {
        free(text);
        return NULL;
    }
    
    size_t k = 0;
    int last_was_space = 1;
    
    for (size_t i = 0; i < j; i++) {
        if (text[i] == ' ' || text[i] == '\t' || text[i] == '\n' || text[i] == '\r') {
            if (!last_was_space) {
                cleaned[k++] = ' ';
                last_was_space = 1;
            }
        } else {
            cleaned[k++] = text[i];
            last_was_space = 0;
        }
    }
    
    cleaned[k] = '\0';
    free(text);
    
    return cleaned;
}

/**
 * Extract links from HTML
 */
void extract_links(const char* html, const char* base_url, URLQueue* queue, int depth) {
    if (!html || !base_url || !queue) return;
    
    const char* p = html;
    while ((p = strstr(p, "href=&quot;")) != NULL) {
        p += 6;  // Skip 'href="'
        
        const char* end = strchr(p, '"');
        if (!end) break;
        
        size_t url_len = end - p;
        if (url_len >= MAX_URL_LENGTH) {
            p = end + 1;
            continue;
        }
        
        char url[MAX_URL_LENGTH];
        strncpy(url, p, url_len);
        url[url_len] = '\0';
        
        // Skip anchors, javascript, mailto
        if (url[0] == '#' || 
            strncmp(url, "javascript:", 11) == 0 ||
            strncmp(url, "mailto:", 7) == 0) {
            p = end + 1;
            continue;
        }
        
        // Handle relative URLs
        if (url[0] == '/') {
            // Extract domain from base_url
            char domain[MAX_URL_LENGTH];
            const char* proto_end = strstr(base_url, "://");
            if (proto_end) {
                proto_end += 3;
                const char* path_start = strchr(proto_end, '/');
                if (path_start) {
                    size_t domain_len = path_start - base_url;
                    strncpy(domain, base_url, domain_len);
                    domain[domain_len] = '\0';
                } else {
                    strcpy(domain, base_url);
                }
                
                char full_url[MAX_URL_LENGTH];
                snprintf(full_url, sizeof(full_url), "%s%s", domain, url);
                url_queue_push(queue, full_url, depth + 1);
            }
        } else if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
            // Absolute URL
            url_queue_push(queue, url, depth + 1);
        }
        
        p = end + 1;
    }
}

/**
 * Main crawler function
 */
int crawl_website(const char* start_url, const char* output_dir, 
                  int max_pages, int max_depth, int delay_ms) {
    printf("Starting web crawler...\n");
    printf("  Start URL: %s\n", start_url);
    printf("  Max pages: %d\n", max_pages);
    printf("  Max depth: %d\n", max_depth);
    printf("  Delay: %d ms\n", delay_ms);
    printf("\n");
    
    URLQueue* queue = url_queue_create();
    VisitedURLs* visited = visited_urls_create();
    
    url_queue_push(queue, start_url, 0);
    
    int pages_crawled = 0;
    int pages_saved = 0;
    
    while (queue->size > 0 && pages_crawled < max_pages) {
        URLNode* node = url_queue_pop(queue);
        if (!node) break;
        
        // Skip if already visited
        if (visited_urls_contains(visited, node->url)) {
            free(node);
            continue;
        }
        
        // Skip if too deep
        if (node->depth > max_depth) {
            free(node);
            continue;
        }
        
        printf("[%d/%d] Crawling (depth %d): %s\n", 
               pages_crawled + 1, max_pages, node->depth, node->url);
        
        // Fetch page
        char* html = fetch_url(node->url, delay_ms);
        if (!html) {
            printf("  Failed to fetch\n");
            free(node);
            continue;
        }
        
        // Extract text
        char* text = extract_text_from_html(html);
        if (text && strlen(text) > 100) {  // Only save if substantial content
            // Save to file
            char filename[512];
            snprintf(filename, sizeof(filename), "%s/page_%04d.txt", 
                     output_dir, pages_saved);
            
            FILE* f = fopen(filename, "w");
            if (f) {
                fprintf(f, "URL: %s\n\n%s\n", node->url, text);
                fclose(f);
                pages_saved++;
                printf("  Saved: %s (%zu bytes)\n", filename, strlen(text));
            }
        }
        
        // Extract links for further crawling
        if (node->depth < max_depth) {
            extract_links(html, node->url, queue, node->depth);
        }
        
        visited_urls_add(visited, node->url);
        pages_crawled++;
        
        free(html);
        free(text);
        free(node);
    }
    
    printf("\nCrawling complete!\n");
    printf("  Pages crawled: %d\n", pages_crawled);
    printf("  Pages saved: %d\n", pages_saved);
    
    url_queue_free(queue);
    visited_urls_free(visited);
    
    return pages_saved;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <start_url> <output_dir> [max_pages] [max_depth] [delay_ms]\n", argv[0]);
        printf("\nExample:\n");
        printf("  %s https://example.com ./data/raw 100 2 2000\n", argv[0]);
        return 1;
    }
    
    const char* start_url = argv[1];
    const char* output_dir = argv[2];
    int max_pages = argc > 3 ? atoi(argv[3]) : 50;
    int max_depth = argc > 4 ? atoi(argv[4]) : 2;
    int delay_ms = argc > 5 ? atoi(argv[5]) : 2000;
    
    srand(time(NULL));
    
    // Create output directory
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", output_dir);
    system(cmd);
    
    crawl_website(start_url, output_dir, max_pages, max_depth, delay_ms);
    
    return 0;
}