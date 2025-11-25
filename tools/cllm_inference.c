/**
 * CLLM Inference Tool
 * 
 * Standalone text generation tool for trained CLLM models.
 */

#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] <model_file>\n\n", program_name);
    printf("Generate text using a trained CLLM model.\n\n");
    printf("Options:\n");
    printf("  -p, --prompt TEXT     Input prompt for generation\n");
    printf("  -f, --file FILE       Read prompt from file\n");
    printf("  -n, --tokens NUM      Number of tokens to generate (default: 100)\n");
    printf("  -t, --temperature T   Sampling temperature (default: 1.0)\n");
    printf("  -k, --top-k K         Top-k sampling (default: 50)\n");
    printf("  -o, --output FILE     Write output to file (default: stdout)\n");
    printf("  -j, --json            Output in JSON format\n");
    printf("  -b, --batch FILE      Batch inference from file (one prompt per line)\n");
    printf("  -s, --seed SEED       Random seed for reproducibility\n");
    printf("  -v, --verbose         Show generation details\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s model.cllm -p &quot;Once upon a time&quot;\n", program_name);
    printf("  %s model.cllm -f prompt.txt -n 200 -t 0.8\n", program_name);
    printf("  %s model.cllm -b prompts.txt -o results.txt\n", program_name);
    printf("  %s model.cllm -p &quot;Hello&quot; -j > output.json\n", program_name);
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

static void generate_text(CLLMModel* model, const char* prompt, 
                         int max_tokens, float temperature, int top_k,
                         bool json_output, bool verbose, FILE* output) {
    if (!model || !prompt) {
        return;
    }

    if (verbose) {
        fprintf(stderr, "Prompt: %s\n", prompt);
        fprintf(stderr, "Generating %d tokens...\n", max_tokens);
    }

    // Tokenize prompt
    int* tokens = malloc(1024 * sizeof(int));
    if (!tokens) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }

    int token_count = cllm_tokenize(prompt, tokens, 1024);
    if (token_count <= 0) {
        fprintf(stderr, "Error: Failed to tokenize prompt\n");
        free(tokens);
        return;
    }

    if (verbose) {
        fprintf(stderr, "Prompt tokens: %d\n", token_count);
    }

    // Generate tokens
    int* generated = malloc((token_count + max_tokens) * sizeof(int));
    if (!generated) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(tokens);
        return;
    }

    memcpy(generated, tokens, token_count * sizeof(int));
    free(tokens);

    int total_tokens = token_count;
    
    for (int i = 0; i < max_tokens; i++) {
        // Get next token prediction
        int next_token = cllm_predict_next_token(model, generated, total_tokens, 
                                                  temperature, top_k);
        if (next_token < 0) {
            break;
        }

        generated[total_tokens++] = next_token;

        if (verbose && (i + 1) % 10 == 0) {
            fprintf(stderr, "Generated %d/%d tokens\r", i + 1, max_tokens);
        }
    }

    if (verbose) {
        fprintf(stderr, "\nTotal tokens generated: %d\n", total_tokens - token_count);
    }

    // Detokenize
    char* output_text = cllm_detokenize(generated, total_tokens);
    if (!output_text) {
        fprintf(stderr, "Error: Failed to detokenize output\n");
        free(generated);
        return;
    }

    // Output
    if (json_output) {
        fprintf(output, "{\n");
        fprintf(output, "  &quot;prompt&quot;: &quot;%s&quot;,\n", prompt);
        fprintf(output, "  &quot;generated_tokens&quot;: %d,\n", total_tokens - token_count);
        fprintf(output, "  &quot;total_tokens&quot;: %d,\n", total_tokens);
        fprintf(output, "  &quot;temperature&quot;: %.2f,\n", temperature);
        fprintf(output, "  &quot;top_k&quot;: %d,\n", top_k);
        fprintf(output, "  &quot;output&quot;: &quot;");
        
        // Escape special characters for JSON
        for (char* p = output_text; *p; p++) {
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
        fprintf(output, "%s", output_text);
        if (output == stdout) {
            fprintf(output, "\n");
        }
    }

    free(output_text);
    free(generated);
}

int main(int argc, char* argv[]) {
    const char* model_path = NULL;
    const char* prompt = NULL;
    const char* prompt_file = NULL;
    const char* output_path = NULL;
    const char* batch_file = NULL;
    int max_tokens = 100;
    float temperature = 1.0f;
    int top_k = 50;
    int seed = -1;
    bool json_output = false;
    bool verbose = false;

    // Parse command-line options
    static struct option long_options[] = {
        {"prompt", required_argument, 0, 'p'},
        {"file", required_argument, 0, 'f'},
        {"tokens", required_argument, 0, 'n'},
        {"temperature", required_argument, 0, 't'},
        {"top-k", required_argument, 0, 'k'},
        {"output", required_argument, 0, 'o'},
        {"json", no_argument, 0, 'j'},
        {"batch", required_argument, 0, 'b'},
        {"seed", required_argument, 0, 's'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "p:f:n:t:k:o:jb:s:vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'p':
                prompt = optarg;
                break;
            case 'f':
                prompt_file = optarg;
                break;
            case 'n':
                max_tokens = atoi(optarg);
                break;
            case 't':
                temperature = atof(optarg);
                break;
            case 'k':
                top_k = atoi(optarg);
                break;
            case 'o':
                output_path = optarg;
                break;
            case 'j':
                json_output = true;
                break;
            case 'b':
                batch_file = optarg;
                break;
            case 's':
                seed = atoi(optarg);
                break;
            case 'v':
                verbose = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Get model file path
    if (optind >= argc) {
        fprintf(stderr, "Error: Model file path required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    model_path = argv[optind];

    // Validate options
    if (!prompt && !prompt_file && !batch_file) {
        fprintf(stderr, "Error: Prompt required (use -p, -f, or -b)\n\n");
        print_usage(argv[0]);
        return 1;
    }

    // Set random seed if specified
    if (seed >= 0) {
        srand(seed);
    }

    // Load model
    if (verbose) {
        fprintf(stderr, "Loading model: %s\n", model_path);
    }

    CLLMModel* model = cllm_load_model(model_path);
    if (!model) {
        fprintf(stderr, "Error: Failed to load model from %s\n", model_path);
        return 1;
    }

    if (verbose) {
        fprintf(stderr, "Model loaded successfully\n");
        fprintf(stderr, "Vocabulary size: %d\n", model->vocab_size);
        fprintf(stderr, "Embedding dimension: %d\n", model->embed_dim);
        fprintf(stderr, "Number of layers: %d\n", model->num_layers);
    }

    // Open output file if specified
    FILE* output = stdout;
    if (output_path) {
        output = fopen(output_path, "w");
        if (!output) {
            fprintf(stderr, "Error: Failed to open output file: %s\n", output_path);
            cllm_free_model(model);
            return 1;
        }
    }

    // Process based on mode
    if (batch_file) {
        // Batch mode
        FILE* batch = fopen(batch_file, "r");
        if (!batch) {
            fprintf(stderr, "Error: Failed to open batch file: %s\n", batch_file);
            if (output != stdout) fclose(output);
            cllm_free_model(model);
            return 1;
        }

        char line[4096];
        int count = 0;
        while (fgets(line, sizeof(line), batch)) {
            // Remove trailing newline
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
            }

            if (strlen(line) > 0) {
                if (verbose) {
                    fprintf(stderr, "\n=== Prompt %d ===\n", ++count);
                }
                generate_text(model, line, max_tokens, temperature, top_k,
                            json_output, verbose, output);
                if (!json_output && output != stdout) {
                    fprintf(output, "\n---\n\n");
                }
            }
        }

        fclose(batch);
    } else {
        // Single prompt mode
        char* prompt_text = NULL;
        
        if (prompt_file) {
            prompt_text = read_file(prompt_file);
            if (!prompt_text) {
                fprintf(stderr, "Error: Failed to read prompt file: %s\n", prompt_file);
                if (output != stdout) fclose(output);
                cllm_free_model(model);
                return 1;
            }
        } else {
            prompt_text = strdup(prompt);
        }

        generate_text(model, prompt_text, max_tokens, temperature, top_k,
                     json_output, verbose, output);

        free(prompt_text);
    }

    // Cleanup
    if (output != stdout) {
        fclose(output);
        if (verbose) {
            fprintf(stderr, "Output written to: %s\n", output_path);
        }
    }

    cllm_free_model(model);

    return 0;
}