/**
 * CLLM Vocabulary Builder Tool
 * 
 * Builds vocabulary from text corpus.
 */

#include "../include/cllm_vocab_builder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] <input_path>\n\n", program_name);
    printf("Build vocabulary from text corpus.\n\n");
    printf("Options:\n");
    printf("  -o, --output FILE     Output vocabulary file (default: vocab.txt)\n");
    printf("  -s, --size NUM        Target vocabulary size (default: 10000)\n");
    printf("  -m, --min-freq NUM    Minimum token frequency (default: 2)\n");
    printf("  -r, --recursive       Process directories recursively\n");
    printf("  -e, --ext EXT         File extension filter (e.g., .txt)\n");
    printf("  -v, --verbose         Show processing details\n");
    printf("  -j, --json            Output statistics in JSON format\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("Input can be:\n");
    printf("  - Single text file\n");
    printf("  - Directory of text files\n");
    printf("  - Multiple files (space-separated)\n\n");
    printf("Examples:\n");
    printf("  %s corpus.txt\n", program_name);
    printf("  %s -r -e .txt data/ -o vocab.txt\n", program_name);
    printf("  %s file1.txt file2.txt file3.txt -s 5000\n", program_name);
}

static bool is_text_file(const char* path, const char* ext_filter) {
    if (!path) return false;
    
    struct stat st;
    if (stat(path, &st) != 0) return false;
    if (!S_ISREG(st.st_mode)) return false;
    
    if (ext_filter) {
        const char* ext = strrchr(path, '.');
        if (!ext || strcmp(ext, ext_filter) != 0) {
            return false;
        }
    }
    
    return true;
}

static int process_file(const char* path, VocabBuilder* builder, bool verbose) {
    if (verbose) {
        printf("Processing: %s\n", path);
    }
    
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Warning: Failed to open %s\n", path);
        return -1;
    }
    
    char line[4096];
    int lines = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (cllm_vocab_add_text(builder, line) == 0) {
            lines++;
        }
    }
    
    fclose(f);
    
    if (verbose) {
        printf("  Processed %d lines\n", lines);
    }
    
    return 0;
}

static int process_directory(const char* path, VocabBuilder* builder, 
                            bool recursive, const char* ext_filter, bool verbose) {
    DIR* dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "Error: Failed to open directory %s\n", path);
        return -1;
    }
    
    struct dirent* entry;
    int file_count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) != 0) continue;
        
        if (S_ISDIR(st.st_mode)) {
            if (recursive) {
                process_directory(full_path, builder, recursive, ext_filter, verbose);
            }
        } else if (S_ISREG(st.st_mode)) {
            if (is_text_file(full_path, ext_filter)) {
                if (process_file(full_path, builder, verbose) == 0) {
                    file_count++;
                }
            }
        }
    }
    
    closedir(dir);
    return file_count;
}

int main(int argc, char* argv[]) {
    const char* output_path = "vocab.txt";
    const char* ext_filter = NULL;
    int vocab_size = 10000;
    int min_freq = 2;
    bool recursive = false;
    bool verbose = false;
    bool json_output = false;

    // Parse command-line options
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"size", required_argument, 0, 's'},
        {"min-freq", required_argument, 0, 'm'},
        {"recursive", no_argument, 0, 'r'},
        {"ext", required_argument, 0, 'e'},
        {"verbose", no_argument, 0, 'v'},
        {"json", no_argument, 0, 'j'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "o:s:m:re:vjh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'o':
                output_path = optarg;
                break;
            case 's':
                vocab_size = atoi(optarg);
                break;
            case 'm':
                min_freq = atoi(optarg);
                break;
            case 'r':
                recursive = true;
                break;
            case 'e':
                ext_filter = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            case 'j':
                json_output = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Check for input paths
    if (optind >= argc) {
        fprintf(stderr, "Error: Input path required\n\n");
        print_usage(argv[0]);
        return 1;
    }

    // Initialize vocabulary builder
    VocabBuilder* builder = cllm_vocab_create(vocab_size);
    if (!builder) {
        fprintf(stderr, "Error: Failed to create vocabulary builder\n");
        return 1;
    }

    if (verbose) {
        printf("=== CLLM Vocabulary Builder ===\n");
        printf("Target vocabulary size: %d\n", vocab_size);
        printf("Minimum frequency: %d\n", min_freq);
        printf("\n");
    }

    // Process all input paths
    int total_files = 0;
    for (int i = optind; i < argc; i++) {
        const char* path = argv[i];
        
        struct stat st;
        if (stat(path, &st) != 0) {
            fprintf(stderr, "Warning: Cannot access %s\n", path);
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            int count = process_directory(path, builder, recursive, ext_filter, verbose);
            if (count > 0) total_files += count;
        } else if (S_ISREG(st.st_mode)) {
            if (process_file(path, builder, verbose) == 0) {
                total_files++;
            }
        }
    }

    if (total_files == 0) {
        fprintf(stderr, "Error: No files processed\n");
        cllm_vocab_free(builder);
        return 1;
    }

    if (verbose) {
        printf("\nTotal files processed: %d\n", total_files);
        printf("Building vocabulary...\n");
    }

    // Build vocabulary with minimum frequency filter
    if (cllm_vocab_build(builder, min_freq) != 0) {
        fprintf(stderr, "Error: Failed to build vocabulary\n");
        cllm_vocab_free(builder);
        return 1;
    }

    // Get statistics
    VocabStats stats = cllm_vocab_get_stats(builder);

    // Save vocabulary
    if (cllm_vocab_save(builder, output_path) != 0) {
        fprintf(stderr, "Error: Failed to save vocabulary to %s\n", output_path);
        cllm_vocab_free(builder);
        return 1;
    }

    // Output results
    if (json_output) {
        printf("{\n");
        printf("  &quot;output_file&quot;: &quot;%s&quot;,\n", output_path);
        printf("  &quot;files_processed&quot;: %d,\n", total_files);
        printf("  &quot;total_tokens&quot;: %d,\n", stats.total_tokens);
        printf("  &quot;unique_tokens&quot;: %d,\n", stats.unique_tokens);
        printf("  &quot;vocab_size&quot;: %d,\n", stats.vocab_size);
        printf("  &quot;coverage&quot;: %.2f\n", stats.coverage * 100);
        printf("}\n");
    } else {
        printf("\n=== Vocabulary Statistics ===\n");
        printf("Files processed: %d\n", total_files);
        printf("Total tokens: %d\n", stats.total_tokens);
        printf("Unique tokens: %d\n", stats.unique_tokens);
        printf("Vocabulary size: %d\n", stats.vocab_size);
        printf("Coverage: %.2f%%\n", stats.coverage * 100);
        printf("\n✓ Vocabulary saved to: %s\n", output_path);
    }

    cllm_vocab_free(builder);
    return 0;
}