/**
 * CLLM Data Loader
 * 
 * Comprehensive data loading and preprocessing for CLLM training
 * Supports:
 * - Text files (.txt)
 * - JSON documents
 * - Web scraping results
 * - PDF extraction (via external tools)
 * - Batch processing
 * - Data augmentation
 */

#include "../include/cllm.h"
#include "../include/cllm_tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_LINE_LENGTH 65536
#define MAX_DOCUMENT_SIZE (100 * 1024 * 1024)  // 100MB max per document

/**
 * Data Loader Structure
 */
typedef struct {
    CLLMTokenizer* tokenizer;
    char** documents;
    size_t num_documents;
    size_t capacity;
    
    // Statistics
    size_t total_chars;
    size_t total_tokens;
    size_t total_lines;
    
    // Configuration
    int min_token_length;
    int max_token_length;
    int lowercase;
    int remove_punctuation;
    int remove_numbers;
} CLLMDataLoader;

/**
 * Create Data Loader
 */
CLLMDataLoader* cllm_data_loader_create(CLLMTokenizer* tokenizer) {
    if (!tokenizer) return NULL;
    
    CLLMDataLoader* loader = (CLLMDataLoader*)calloc(1, sizeof(CLLMDataLoader));
    if (!loader) return NULL;
    
    loader->tokenizer = tokenizer;
    loader->capacity = 1000;
    loader->documents = (char**)calloc(loader->capacity, sizeof(char*));
    
    if (!loader->documents) {
        free(loader);
        return NULL;
    }
    
    // Default configuration
    loader->min_token_length = 1;
    loader->max_token_length = 50;
    loader->lowercase = 1;
    loader->remove_punctuation = 0;
    loader->remove_numbers = 0;
    
    return loader;
}

/**
 * Free Data Loader
 */
void cllm_data_loader_free(CLLMDataLoader* loader) {
    if (!loader) return;
    
    if (loader->documents) {
        for (size_t i = 0; i < loader->num_documents; i++) {
            free(loader->documents[i]);
        }
        free(loader->documents);
    }
    
    free(loader);
}

/**
 * Clean Text
 * Applies preprocessing rules
 */
static char* clean_text(const char* text, CLLMDataLoader* loader) {
    if (!text) return NULL;
    
    size_t len = strlen(text);
    char* cleaned = (char*)malloc(len + 1);
    if (!cleaned) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = text[i];
        
        // Convert to lowercase if enabled
        if (loader->lowercase) {
            c = tolower(c);
        }
        
        // Remove punctuation if enabled
        if (loader->remove_punctuation && ispunct(c)) {
            continue;
        }
        
        // Remove numbers if enabled
        if (loader->remove_numbers && isdigit(c)) {
            continue;
        }
        
        // Normalize whitespace
        if (isspace(c)) {
            // Skip multiple spaces
            if (j > 0 && cleaned[j-1] == ' ') {
                continue;
            }
            c = ' ';
        }
        
        cleaned[j++] = c;
    }
    
    cleaned[j] = '\0';
    return cleaned;
}

/**
 * Add Document
 */
int cllm_data_loader_add_document(CLLMDataLoader* loader, const char* text) {
    if (!loader || !text) return 0;
    
    // Expand capacity if needed
    if (loader->num_documents >= loader->capacity) {
        size_t new_capacity = loader->capacity * 2;
        char** new_docs = (char**)realloc(loader->documents, 
                                          new_capacity * sizeof(char*));
        if (!new_docs) return 0;
        
        loader->documents = new_docs;
        loader->capacity = new_capacity;
    }
    
    // Clean and store document
    char* cleaned = clean_text(text, loader);
    if (!cleaned) return 0;
    
    loader->documents[loader->num_documents++] = cleaned;
    loader->total_chars += strlen(cleaned);
    
    // Count lines
    for (const char* p = cleaned; *p; p++) {
        if (*p == '\n') loader->total_lines++;
    }
    
    return 1;
}

/**
 * Load Text File
 */
int cllm_data_loader_load_file(CLLMDataLoader* loader, const char* filename) {
    if (!loader || !filename) return 0;
    
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return 0;
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (file_size > MAX_DOCUMENT_SIZE) {
        fprintf(stderr, "File too large: %s (%ld bytes)\n", filename, file_size);
        fclose(f);
        return 0;
    }
    
    // Read entire file
    char* content = (char*)malloc(file_size + 1);
    if (!content) {
        fclose(f);
        return 0;
    }
    
    size_t bytes_read = fread(content, 1, file_size, f);
    content[bytes_read] = '\0';
    fclose(f);
    
    // Add as document
    int result = cllm_data_loader_add_document(loader, content);
    free(content);
    
    if (result) {
        printf("Loaded: %s (%ld bytes)\n", filename, file_size);
    }
    
    return result;
}

/**
 * Load Directory
 * Recursively loads all .txt files from directory
 */
int cllm_data_loader_load_directory(CLLMDataLoader* loader, const char* dirname) {
    if (!loader || !dirname) return 0;
    
    DIR* dir = opendir(dirname);
    if (!dir) {
        fprintf(stderr, "Failed to open directory: %s\n", dirname);
        return 0;
    }
    
    int count = 0;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Build full path
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
        
        // Check if it's a directory
        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                // Recursively load subdirectory
                count += cllm_data_loader_load_directory(loader, path);
            } else if (S_ISREG(st.st_mode)) {
                   // Load ALL file types (not just .txt)
                   // Skip binary files and hidden files
                   if (entry->d_name[0] != '.') {
                       // Skip common binary extensions
                       const char* ext = strrchr(entry->d_name, '.');
                       int is_binary = 0;
                       if (ext) {
                           if (strcmp(ext, ".o") == 0 || strcmp(ext, ".so") == 0 || 
                               strcmp(ext, ".a") == 0 || strcmp(ext, ".bin") == 0 ||
                               strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
                               strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
                               strcmp(ext, ".gif") == 0 || strcmp(ext, ".pdf") == 0) {
                               is_binary = 1;
                           }
                       }
                       if (!is_binary && cllm_data_loader_load_file(loader, path)) {
                           count++;
                       }
                   }
                   // Load ALL file types (not just .txt)
                   // Skip binary files and hidden files
                   if (entry->d_name[0] != '.') {
                       // Skip common binary extensions
                       const char* ext = strrchr(entry->d_name, '.');
                       int is_binary = 0;
                       if (ext) {
                           if (strcmp(ext, ".o") == 0 || strcmp(ext, ".so") == 0 || 
                               strcmp(ext, ".a") == 0 || strcmp(ext, ".bin") == 0 ||
                               strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
                               strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
                               strcmp(ext, ".gif") == 0 || strcmp(ext, ".pdf") == 0) {
                               is_binary = 1;
                           }
                       }
                       if (!is_binary && cllm_data_loader_load_file(loader, path)) {
                           count++;
                       }
                   }
                   // Load ALL file types (not just .txt)
                   // Skip binary files and hidden files
                   if (entry->d_name[0] != '.') {
                       // Skip common binary extensions
                       const char* ext = strrchr(entry->d_name, '.');
                       int is_binary = 0;
                       if (ext) {
                           if (strcmp(ext, ".o") == 0 || strcmp(ext, ".so") == 0 || 
                               strcmp(ext, ".a") == 0 || strcmp(ext, ".bin") == 0 ||
                               strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
                               strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
                               strcmp(ext, ".gif") == 0 || strcmp(ext, ".pdf") == 0) {
                               is_binary = 1;
                           }
                       }
                       if (!is_binary && cllm_data_loader_load_file(loader, path)) {
                           count++;
                       }
                   }
                   // Load ALL file types (not just .txt)
                   // Skip binary files and hidden files
                   if (entry->d_name[0] != '.') {
                       // Skip common binary extensions
                       const char* ext = strrchr(entry->d_name, '.');
                       int is_binary = 0;
                       if (ext) {
                           if (strcmp(ext, ".o") == 0 || strcmp(ext, ".so") == 0 || 
                               strcmp(ext, ".a") == 0 || strcmp(ext, ".bin") == 0 ||
                               strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
                               strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
                               strcmp(ext, ".gif") == 0 || strcmp(ext, ".pdf") == 0) {
                               is_binary = 1;
                           }
                       }
                       if (!is_binary && cllm_data_loader_load_file(loader, path)) {
                           count++;
                       }
                   }
                   // Load ALL file types (not just .txt)
                   // Skip binary files and hidden files
                   if (entry->d_name[0] != '.') {
                       // Skip common binary extensions
                       const char* ext = strrchr(entry->d_name, '.');
                       int is_binary = 0;
                       if (ext) {
                           if (strcmp(ext, ".o") == 0 || strcmp(ext, ".so") == 0 || 
                               strcmp(ext, ".a") == 0 || strcmp(ext, ".bin") == 0 ||
                               strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
                               strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
                               strcmp(ext, ".gif") == 0 || strcmp(ext, ".pdf") == 0) {
                               is_binary = 1;
                           }
                       }
                       if (!is_binary && cllm_data_loader_load_file(loader, path)) {
                           count++;
                       }
                   }
                   // Load ALL file types (not just .txt)
                   // Skip binary files and hidden files
                   if (entry->d_name[0] != '.') {
                       // Skip common binary extensions
                       const char* ext = strrchr(entry->d_name, '.');
                       int is_binary = 0;
                       if (ext) {
                           if (strcmp(ext, ".o") == 0 || strcmp(ext, ".so") == 0 || 
                               strcmp(ext, ".a") == 0 || strcmp(ext, ".bin") == 0 ||
                               strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
                               strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
                               strcmp(ext, ".gif") == 0 || strcmp(ext, ".pdf") == 0) {
                               is_binary = 1;
                           }
                       }
                       if (!is_binary && cllm_data_loader_load_file(loader, path)) {
                           count++;
                       }
                   }
                   // Load ALL file types (not just .txt)
                   // Skip binary files and hidden files
                   if (entry->d_name[0] != '.') {
                       // Skip common binary extensions
                       const char* ext = strrchr(entry->d_name, '.');
                       int is_binary = 0;
                       if (ext) {
                           if (strcmp(ext, ".o") == 0 || strcmp(ext, ".so") == 0 || 
                               strcmp(ext, ".a") == 0 || strcmp(ext, ".bin") == 0 ||
                               strcmp(ext, ".exe") == 0 || strcmp(ext, ".dll") == 0 ||
                               strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
                               strcmp(ext, ".gif") == 0 || strcmp(ext, ".pdf") == 0) {
                               is_binary = 1;
                           }
                       }
                       if (!is_binary && cllm_data_loader_load_file(loader, path)) {
                           count++;
                       }
                   }
            }
        }
    }
    
    closedir(dir);
    return count;
}

/**
 * Build Vocabulary from Loaded Documents
 */
void cllm_data_loader_build_vocab(CLLMDataLoader* loader) {
    if (!loader || !loader->tokenizer) return;
    
    printf("Building vocabulary from %zu documents...\n", loader->num_documents);
    
    for (size_t i = 0; i < loader->num_documents; i++) {
        cllm_build_vocab(loader->tokenizer, loader->documents[i]);
    }
    
    loader->total_tokens = 0;
    for (uint32_t i = 0; i < loader->tokenizer->vocab_size; i++) {
        loader->total_tokens += loader->tokenizer->token_counts[i];
    }
    
    printf("Vocabulary built: %u unique tokens, %zu total tokens\n",
           loader->tokenizer->vocab_size, loader->total_tokens);
}

/**
 * Create Training Dataset
 * Tokenizes all documents and creates training sequences
 */
typedef struct {
    uint32_t* tokens;
    size_t num_tokens;
    size_t capacity;
} TokenDataset;

TokenDataset* cllm_data_loader_create_dataset(CLLMDataLoader* loader) {
    if (!loader || !loader->tokenizer) return NULL;
    
    TokenDataset* dataset = (TokenDataset*)calloc(1, sizeof(TokenDataset));
    if (!dataset) return NULL;
    
    // Estimate capacity
    dataset->capacity = loader->total_chars / 4;  // Rough estimate
    dataset->tokens = (uint32_t*)malloc(dataset->capacity * sizeof(uint32_t));
    
    if (!dataset->tokens) {
        free(dataset);
        return NULL;
    }
    
    printf("Creating training dataset...\n");
    
    // Tokenize all documents
    for (size_t i = 0; i < loader->num_documents; i++) {
        uint32_t num_tokens;
        uint32_t* doc_tokens = cllm_tokenizer_encode(loader->tokenizer, 
                                                     loader->documents[i], 
                                                     &num_tokens);
        
        if (doc_tokens) {
            // Expand capacity if needed
            while (dataset->num_tokens + num_tokens > dataset->capacity) {
                dataset->capacity *= 2;
                uint32_t* new_tokens = (uint32_t*)realloc(dataset->tokens,
                                                          dataset->capacity * sizeof(uint32_t));
                if (!new_tokens) {
                    free(doc_tokens);
                    free(dataset->tokens);
                    free(dataset);
                    return NULL;
                }
                dataset->tokens = new_tokens;
            }
            
            // Copy tokens
            memcpy(dataset->tokens + dataset->num_tokens, doc_tokens, 
                   num_tokens * sizeof(uint32_t));
            dataset->num_tokens += num_tokens;
            
            free(doc_tokens);
        }
        
        if ((i + 1) % 100 == 0) {
            printf("  Processed %zu/%zu documents\n", i + 1, loader->num_documents);
        }
    }
    
    printf("Dataset created: %zu tokens\n", dataset->num_tokens);
    return dataset;
}

/**
 * Free Token Dataset
 */
void cllm_token_dataset_free(TokenDataset* dataset) {
    if (!dataset) return;
    free(dataset->tokens);
    free(dataset);
}

/**
 * Save Dataset to File
 */
int cllm_token_dataset_save(TokenDataset* dataset, const char* filename) {
    if (!dataset || !filename) return 0;
    
    FILE* f = fopen(filename, "wb");
    if (!f) return 0;
    
    // Write header
    fwrite(&dataset->num_tokens, sizeof(size_t), 1, f);
    
    // Write tokens
    fwrite(dataset->tokens, sizeof(uint32_t), dataset->num_tokens, f);
    
    fclose(f);
    
    printf("Dataset saved to: %s (%zu tokens)\n", filename, dataset->num_tokens);
    return 1;
}

/**
 * Load Dataset from File
 */
TokenDataset* cllm_token_dataset_load(const char* filename) {
    if (!filename) return NULL;
    
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;
    
    TokenDataset* dataset = (TokenDataset*)calloc(1, sizeof(TokenDataset));
    if (!dataset) {
        fclose(f);
        return NULL;
    }
    
    // Read header
    if (fread(&dataset->num_tokens, sizeof(size_t), 1, f) != 1) {
        free(dataset);
        fclose(f);
        return NULL;
    }
    
    // Allocate and read tokens
    dataset->capacity = dataset->num_tokens;
    dataset->tokens = (uint32_t*)malloc(dataset->capacity * sizeof(uint32_t));
    
    if (!dataset->tokens) {
        free(dataset);
        fclose(f);
        return NULL;
    }
    
    if (fread(dataset->tokens, sizeof(uint32_t), dataset->num_tokens, f) != dataset->num_tokens) {
        free(dataset->tokens);
        free(dataset);
        fclose(f);
        return NULL;
    }
    
    fclose(f);
    
    printf("Dataset loaded from: %s (%zu tokens)\n", filename, dataset->num_tokens);
    return dataset;
}

/**
 * Print Dataset Statistics
 */
void cllm_data_loader_print_stats(CLLMDataLoader* loader) {
    if (!loader) return;
    
    printf("\n=== Data Loader Statistics ===\n");
    printf("Documents: %zu\n", loader->num_documents);
    printf("Total characters: %zu\n", loader->total_chars);
    printf("Total lines: %zu\n", loader->total_lines);
    printf("Total tokens: %zu\n", loader->total_tokens);
    
    if (loader->num_documents > 0) {
        printf("Avg chars per document: %.1f\n", 
               (double)loader->total_chars / loader->num_documents);
    }
    
    if (loader->tokenizer) {
        printf("Vocabulary size: %u\n", loader->tokenizer->vocab_size);
    }
    
    printf("==============================\n\n");
}