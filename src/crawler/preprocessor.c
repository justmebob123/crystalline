/**
 * HTML Preprocessor
 * 
 * Converts HTML pages to clean text suitable for training
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define MAX_TEXT_SIZE (5 * 1024 * 1024)  // 5MB max text
#define MIN_TEXT_LENGTH 100

typedef struct {
    char data_dir[1024];
    int running;
    int files_processed;
    pthread_mutex_t lock;
} PreprocessorState;

/**
 * Remove HTML tags from text
 */
static void remove_html_tags(const char* html, char* text, size_t text_size) {
    const char* p = html;
    char* out = text;
    char* out_end = text + text_size - 1;
    int in_tag = 0;
    int in_script = 0;
    int in_style = 0;
    
    while (*p && out < out_end) {
        // Check for script/style tags
        if (strncasecmp(p, "<script", 7) == 0) {
            in_script = 1;
            in_tag = 1;
        } else if (strncasecmp(p, "</script>", 9) == 0) {
            in_script = 0;
            p += 9;
            continue;
        } else if (strncasecmp(p, "<style", 6) == 0) {
            in_style = 1;
            in_tag = 1;
        } else if (strncasecmp(p, "</style>", 8) == 0) {
            in_style = 0;
            p += 8;
            continue;
        }
        
        // Skip script/style content
        if (in_script || in_style) {
            p++;
            continue;
        }
        
        // Handle tags
        if (*p == '<') {
            in_tag = 1;
            p++;
            continue;
        } else if (*p == '>') {
            in_tag = 0;
            p++;
            // Add space after tag
            if (out > text && *(out-1) != ' ' && *(out-1) != '\n') {
                *out++ = ' ';
            }
            continue;
        }
        
        // Skip tag content
        if (in_tag) {
            p++;
            continue;
        }
        
        // Copy text content
        *out++ = *p++;
    }
    
    *out = '\0';
}

/**
 * Clean and normalize text
 */
static void clean_text(char* text) {
    char* src = text;
    char* dst = text;
    int last_was_space = 1;
    
    while (*src) {
        // Convert HTML entities (simple version)
        if (strncmp(src, "&nbsp;", 6) == 0) {
            *dst++ = ' ';
            src += 6;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&lt;", 4) == 0) {
            *dst++ = '<';
            src += 4;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&gt;", 4) == 0) {
            *dst++ = '>';
            src += 4;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&amp;", 5) == 0) {
            *dst++ = '&';
            src += 5;
            last_was_space = 0;
            continue;
        } else if (strncmp(src, "&quot;", 6) == 0) {
            *dst++ = '"';
            src += 6;
            last_was_space = 0;
            continue;
        }
        
        // Normalize whitespace
        if (isspace(*src)) {
            if (!last_was_space) {
                *dst++ = ' ';
                last_was_space = 1;
            }
            src++;
        } else {
            *dst++ = *src++;
            last_was_space = 0;
        }
    }
    
    *dst = '\0';
    
    // Trim leading/trailing whitespace
    while (*text && isspace(*text)) text++;
    
    size_t len = strlen(text);
    while (len > 0 && isspace(text[len-1])) {
        text[len-1] = '\0';
        len--;
    }
}

/**
 * Process one HTML file
 */
static int preprocess_file(const char* input_path, const char* output_path) {
    // Read input file
    FILE* f = fopen(input_path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open: %s\n", input_path);
        return -1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* html = (char*)malloc(size + 1);
    if (!html) {
        fclose(f);
        return -1;
    }
    
    fread(html, 1, size, f);
    html[size] = '\0';
    fclose(f);
    
    // Extract text
    char* text = (char*)malloc(MAX_TEXT_SIZE);
    if (!text) {
        free(html);
        return -1;
    }
    
    remove_html_tags(html, text, MAX_TEXT_SIZE);
    free(html);
    
    // Clean text
    clean_text(text);
    
    // Check minimum length
    if (strlen(text) < MIN_TEXT_LENGTH) {
        printf("  Skipped (too short): %zu chars\n", strlen(text));
        free(text);
        return -1;
    }
    
    // Write output
    FILE* out = fopen(output_path, "w");
    if (!out) {
        fprintf(stderr, "Failed to write: %s\n", output_path);
        free(text);
        return -1;
    }
    
    fprintf(out, "%s\n", text);
    fclose(out);
    free(text);
    
    return 0;
}

/**
 * Preprocessor thread
 */
/**
 * Get current timestamp string
 */
static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "[%H:%M:%S]", tm_info);
}

void* preprocessor_thread_func(void* arg) {
    PreprocessorState* state = (PreprocessorState*)arg;
    char timestamp[32];
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === PREPROCESSOR STARTED ===\n", timestamp);
    
    char raw_dir[1024];
    char preprocessed_dir[1024];
    snprintf(raw_dir, sizeof(raw_dir), "%s/raw_pages", state->data_dir);
    snprintf(preprocessed_dir, sizeof(preprocessed_dir), "%s/preprocessed", state->data_dir);
    
    while (state->running) {
        DIR* dir = opendir(raw_dir);
        if (!dir) {
            sleep(5);
            continue;
        }
        
        struct dirent* entry;
        int found_file = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            if (strstr(entry->d_name, ".html") == NULL) continue;
            
            // Check if already processed
            char preprocessed_path[2048];
            char* base = strdup(entry->d_name);
            char* dot = strrchr(base, '.');
            if (dot) *dot = '\0';
            
            snprintf(preprocessed_path, sizeof(preprocessed_path), 
                    "%s/%s.txt", preprocessed_dir, base);
            
            // Check if output exists
            if (access(preprocessed_path, F_OK) == 0) {
                free(base);
                continue;
            }
            
            // Process file
            char input_path[2048];
            snprintf(input_path, sizeof(input_path), "%s/%s", raw_dir, entry->d_name);
            
            get_timestamp(timestamp, sizeof(timestamp));
            printf("%s Preprocessing: %s\n", timestamp, entry->d_name);
            
            if (preprocess_file(input_path, preprocessed_path) == 0) {
                get_timestamp(timestamp, sizeof(timestamp));
                printf("%s ✓ Preprocessed: %s\n", timestamp, base);
                pthread_mutex_lock(&state->lock);
                state->files_processed++;
                pthread_mutex_unlock(&state->lock);
            }
            
            free(base);
            found_file = 1;
            break;  // Process one file at a time
        }
        
        closedir(dir);
        
        if (!found_file) {
            sleep(5);  // Wait for new files
        } else {
            sleep(1);  // Small delay between files
        }
    }
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s === PREPROCESSOR STOPPED ===\n", timestamp);
    return NULL;
}

/**
 * Initialize preprocessor
 */
PreprocessorState* preprocessor_init(const char* data_dir) {
    PreprocessorState* state = (PreprocessorState*)calloc(1, sizeof(PreprocessorState));
    if (!state) return NULL;
    
    strncpy(state->data_dir, data_dir, sizeof(state->data_dir) - 1);
    state->running = 1;
    state->files_processed = 0;
    pthread_mutex_init(&state->lock, NULL);
    
    return state;
}

/**
 * Cleanup preprocessor
 */
void preprocessor_cleanup(PreprocessorState* state) {
    if (!state) return;
    pthread_mutex_destroy(&state->lock);
    free(state);
}