/**
 * CLLM Tokenizer Tool
 * 
 * Standalone tokenization tool for text processing.
 */

#include "../include/cllm_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] [text]\n\n", program_name);
    printf("Tokenize text using CLLM tokenizer.\n\n");
    printf("Options:\n");
    printf("  -f, --file FILE       Read text from file\n");
    printf("  -o, --output FILE     Write output to file (default: stdout)\n");
    printf("  -d, --decode          Decode token IDs to text\n");
    printf("  -s, --stats           Show tokenization statistics\n");
    printf("  -v, --vocab FILE      Use custom vocabulary file\n");
    printf("  -j, --json            Output in JSON format\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s &quot;Hello, world!&quot;\n", program_name);
    printf("  %s -f input.txt -o tokens.txt\n", program_name);
    printf("  %s -d &quot;42 123 456&quot;\n", program_name);
    printf("  %s -f input.txt -s -j\n", program_name);
}

static char* read_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = malloc(size + 1);
    if (!content) {
        fclose(f);
        return NULL;
    }

    size_t read = fread(content, 1, size, f);
    content[read] = '\0';
    fclose(f);

    return content;
}

static void tokenize_text(const char* text, bool show_stats, bool json_output, FILE* output) {
    if (!text) return;

    // Tokenize
    int* tokens = malloc(strlen(text) * sizeof(int));
    if (!tokens) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }

    int token_count = cllm_tokenize(text, tokens, strlen(text));
    if (token_count <= 0) {
        fprintf(stderr, "Error: Tokenization failed\n");
        free(tokens);
        return;
    }

    // Calculate statistics
    int unique_tokens = 0;
    int max_token = 0;
    int min_token = tokens[0];
    
    if (show_stats) {
        // Count unique tokens (simple approach)
        for (int i = 0; i < token_count; i++) {
            if (tokens[i] > max_token) max_token = tokens[i];
            if (tokens[i] < min_token) min_token = tokens[i];
        }
        
        // Estimate unique count (would need hash set for exact count)
        unique_tokens = (max_token - min_token + 1);
    }

    // Output
    if (json_output) {
        fprintf(output, "{\n");
        fprintf(output, "  &quot;text_length&quot;: %zu,\n", strlen(text));
        fprintf(output, "  &quot;token_count&quot;: %d,\n", token_count);
        
        if (show_stats) {
            fprintf(output, "  &quot;min_token_id&quot;: %d,\n", min_token);
            fprintf(output, "  &quot;max_token_id&quot;: %d,\n", max_token);
            fprintf(output, "  &quot;compression_ratio&quot;: %.2f,\n", 
                   (float)strlen(text) / token_count);
        }
        
        fprintf(output, "  &quot;tokens&quot;: [");
        for (int i = 0; i < token_count; i++) {
            fprintf(output, "%d", tokens[i]);
            if (i < token_count - 1) fprintf(output, ", ");
        }
        fprintf(output, "]\n}\n");
    } else {
        if (show_stats) {
            fprintf(output, "=== Tokenization Statistics ===\n");
            fprintf(output, "Text length: %zu characters\n", strlen(text));
            fprintf(output, "Token count: %d tokens\n", token_count);
            fprintf(output, "Compression ratio: %.2f chars/token\n", 
                   (float)strlen(text) / token_count);
            fprintf(output, "Token ID range: %d - %d\n", min_token, max_token);
            fprintf(output, "\n=== Tokens ===\n");
        }
        
        for (int i = 0; i < token_count; i++) {
            fprintf(output, "%d", tokens[i]);
            if (i < token_count - 1) fprintf(output, " ");
        }
        fprintf(output, "\n");
    }

    free(tokens);
}

static void decode_tokens(const char* token_str, bool json_output, FILE* output) {
    if (!token_str) return;

    // Parse token IDs
    int* tokens = malloc(strlen(token_str) * sizeof(int));
    if (!tokens) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }

    int token_count = 0;
    char* str_copy = strdup(token_str);
    char* token = strtok(str_copy, " ,\t\n");
    
    while (token && token_count < (int)strlen(token_str)) {
        tokens[token_count++] = atoi(token);
        token = strtok(NULL, " ,\t\n");
    }
    
    free(str_copy);

    if (token_count == 0) {
        fprintf(stderr, "Error: No valid token IDs found\n");
        free(tokens);
        return;
    }

    // Detokenize
    char* text = cllm_detokenize(tokens, token_count);
    if (!text) {
        fprintf(stderr, "Error: Detokenization failed\n");
        free(tokens);
        return;
    }

    // Output
    if (json_output) {
        fprintf(output, "{\n");
        fprintf(output, "  &quot;token_count&quot;: %d,\n", token_count);
        fprintf(output, "  &quot;text&quot;: &quot;");
        
        // Escape special characters for JSON
        for (char* p = text; *p; p++) {
            if (*p == '"' || *p == '\\') {
                fputc('\\', output);
            }
            if (*p == '\n') {
                fprintf(output, "\\n");
            } else if (*p == '\t') {
                fprintf(output, "\\t");
            } else {
                fputc(*p, output);
            }
        }
        
        fprintf(output, "&quot;\n}\n");
    } else {
        fprintf(output, "%s\n", text);
    }

    free(text);
    free(tokens);
}

int main(int argc, char* argv[]) {
    const char* input_file = NULL;
    const char* output_path = NULL;
    const char* vocab_file = NULL;
    const char* text = NULL;
    bool decode_mode = false;
    bool show_stats = false;
    bool json_output = false;

    // Parse command-line options
    static struct option long_options[] = {
        {"file", required_argument, 0, 'f'},
        {"output", required_argument, 0, 'o'},
        {"decode", no_argument, 0, 'd'},
        {"stats", no_argument, 0, 's'},
        {"vocab", required_argument, 0, 'v'},
        {"json", no_argument, 0, 'j'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "f:o:dsv:jh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'f':
                input_file = optarg;
                break;
            case 'o':
                output_path = optarg;
                break;
            case 'd':
                decode_mode = true;
                break;
            case 's':
                show_stats = true;
                break;
            case 'v':
                vocab_file = optarg;
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

    // Get text from command line if not from file
    if (!input_file && optind < argc) {
        text = argv[optind];
    }

    // Validate input
    if (!input_file && !text) {
        fprintf(stderr, "Error: Input text required (use -f or provide text)\n\n");
        print_usage(argv[0]);
        return 1;
    }

    // Load custom vocabulary if specified
    if (vocab_file) {
        if (cllm_load_vocabulary(vocab_file) != 0) {
            fprintf(stderr, "Error: Failed to load vocabulary from %s\n", vocab_file);
            return 1;
        }
    }

    // Read input file if specified
    char* input_text = NULL;
    if (input_file) {
        input_text = read_file(input_file);
        if (!input_text) {
            fprintf(stderr, "Error: Failed to read input file: %s\n", input_file);
            return 1;
        }
        text = input_text;
    }

    // Open output file if specified
    FILE* output = stdout;
    if (output_path) {
        output = fopen(output_path, "w");
        if (!output) {
            fprintf(stderr, "Error: Failed to open output file: %s\n", output_path);
            if (input_text) free(input_text);
            return 1;
        }
    }

    // Process
    if (decode_mode) {
        decode_tokens(text, json_output, output);
    } else {
        tokenize_text(text, show_stats, json_output, output);
    }

    // Cleanup
    if (input_text) free(input_text);
    if (output != stdout) fclose(output);

    return 0;
}