/**
 * CLLM Inference Tool - Proper Implementation
 * 
 * Uses the actual model forward pass instead of random logits
 */

#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "../include/prime_float_math.h"

// Forward declarations from cllm_inference.c
void cllm_forward(CLLMInference* inference, uint32_t* tokens, int num_tokens);
void cllm_apply_temperature(float* logits, int vocab_size, float temperature);
void cllm_softmax(float* logits, int size);
uint32_t cllm_sample_top_k(float* logits, int vocab_size, int k);

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] <model_file> <vocab_file>\n\n", program_name);
    printf("Generate text using a trained CLLM model.\n\n");
    printf("Options:\n");
    printf("  -p, --prompt TEXT     Input prompt for generation\n");
    printf("  -n, --tokens NUM      Number of tokens to generate (default: 50)\n");
    printf("  -t, --temperature T   Sampling temperature (default: 0.8)\n");
    printf("  -k, --top-k K         Top-k sampling (default: 40)\n");
    printf("  -s, --seed SEED       Random seed for reproducibility\n");
    printf("  -v, --verbose         Show generation details\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s model.cllm vocab.txt -p &quot;int main&quot;\n", program_name);
    printf("  %s model.cllm vocab.txt -p &quot;struct&quot; -n 100 -t 0.5\n", program_name);
}

static void generate_text_proper(CLLMInference* inference, const char* prompt,
                                 int max_tokens, float temperature, int top_k,
                                 bool verbose) {
    if (!inference || !prompt) {
        return;
    }
    
    CLLMModel* model = inference->model;
    
    if (verbose) {
        fprintf(stderr, "\n=== Generation Parameters ===\n");
        fprintf(stderr, "Prompt: &quot;%s&quot;\n", prompt);
        fprintf(stderr, "Max tokens: %d\n", max_tokens);
        fprintf(stderr, "Temperature: %.2f\n", temperature);
        fprintf(stderr, "Top-k: %d\n", top_k);
        fprintf(stderr, "Model vocab size: %lu\n", (unsigned long)model->vocab_size);
        fprintf(stderr, "============================\n\n");
    }
    
    // Tokenize prompt
    uint32_t tokens[1024];
    int num_tokens = cllm_tokenize(inference, prompt, tokens, 1024);
    
    if (num_tokens <= 0) {
        fprintf(stderr, "Error: Failed to tokenize prompt\n");
        return;
    }
    
    if (verbose) {
        fprintf(stderr, "Tokenized prompt: %d tokens\n", num_tokens);
        fprintf(stderr, "Token IDs: ");
        for (int i = 0; i < num_tokens && i < 10; i++) {
            fprintf(stderr, "%u ", tokens[i]);
        }
        if (num_tokens > 10) fprintf(stderr, "...");
        fprintf(stderr, "\n\n");
    }
    
    // Print prompt
    printf("Prompt: %s\n", prompt);
    printf("Generated: ");
    fflush(stdout);
    
    // Generate tokens
    int generated_count = 0;
    for (int i = 0; i < max_tokens; i++) {
        // Run forward pass through the model
        cllm_forward(inference, tokens, num_tokens);
        
        // Check if logits were computed
        if (!inference->logits) {
            fprintf(stderr, "\nError: Forward pass did not produce logits\n");
            break;
        }
        
        // Apply temperature
        if (temperature > 0.0f && temperature != 1.0f) {
            cllm_apply_temperature(inference->logits, model->vocab_size, temperature);
        }
        
        // Apply softmax
        cllm_softmax(inference->logits, model->vocab_size);
        
        // Sample next token
        uint32_t next_token;
        if (top_k > 1 && temperature > 0.0f) {
            next_token = cllm_sample_top_k(inference->logits, model->vocab_size, top_k);
        } else {
            // Greedy sampling - pick highest probability
            next_token = (uint32_t)cllm_sample_token(inference, inference->logits);
        }
        
        // Check validity
        if (next_token >= model->vocab_size) {
            if (verbose) {
                fprintf(stderr, "\nWarning: Invalid token %u generated (vocab_size=%lu)\n", 
                       next_token, (unsigned long)model->vocab_size);
            }
            break;
        }
        
        // Add to sequence
        if (num_tokens < 1024) {
            tokens[num_tokens++] = next_token;
            generated_count++;
        } else {
            if (verbose) {
                fprintf(stderr, "\nWarning: Reached maximum sequence length\n");
            }
            break;
        }
        
        // Detokenize and print this token
        char token_text[256];
        cllm_detokenize(inference, &next_token, 1, token_text, 256);
        printf("%s", token_text);
        fflush(stdout);
        
        if (verbose && (i + 1) % 10 == 0) {
            fprintf(stderr, "\r[Generated %d/%d tokens]", i + 1, max_tokens);
        }
    }
    
    printf("\n");
    
    if (verbose) {
        fprintf(stderr, "\n\n=== Generation Complete ===\n");
        fprintf(stderr, "Total tokens generated: %d\n", generated_count);
        fprintf(stderr, "Final sequence length: %d\n", num_tokens);
        fprintf(stderr, "==========================\n");
    }
}

int main(int argc, char** argv) {
    // Default parameters
    const char* model_path = NULL;
    const char* vocab_path = NULL;
    const char* prompt = "int main";
    int max_tokens = 50;
    float temperature = 0.8f;
    int top_k = 40;
    int seed = -1;
    bool verbose = false;
    
    // Parse command line options
    static struct option long_options[] = {
        {"prompt", required_argument, 0, 'p'},
        {"tokens", required_argument, 0, 'n'},
        {"temperature", required_argument, 0, 't'},
        {"top-k", required_argument, 0, 'k'},
        {"seed", required_argument, 0, 's'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "p:n:t:k:s:vh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                prompt = optarg;
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
    
    // Get positional arguments
    if (optind + 2 > argc) {
        fprintf(stderr, "Error: Missing required arguments\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    model_path = argv[optind];
    vocab_path = argv[optind + 1];
    
    // Set random seed
    if (seed >= 0) {
        srand(seed);
    } else {
        srand(time(NULL));
    }
    
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║    CLLM Inference Engine v2.0 (Proper Forward Pass)     ║\n");
    printf("║         Crystalline Lattice Language Model              ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    
    // Load model
    if (verbose) {
        printf("Loading model from: %s\n", model_path);
    }
    
    CLLMModel* model = cllm_read_model(model_path);
    if (!model) {
        fprintf(stderr, "Error: Failed to load model from %s\n", model_path);
        return 1;
    }
    
    if (verbose) {
        printf("✓ Model loaded successfully\n");
        printf("  Vocabulary size: %lu\n", (unsigned long)model->vocab_size);
        printf("  Embedding dim:   %lu\n", (unsigned long)model->embedding_dim);
        printf("  Num layers:      %u\n", model->num_layers);
        printf("  Num heads:       %u\n", model->header.num_heads);
        printf("\n");
    }
    
    // Note: vocab_path provided but not currently used
    (void)vocab_path;
    
    // Initialize inference engine
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Error: Failed to initialize inference engine\n");
        cllm_free(model);
        return 1;
    }
    
    // Set inference parameters
    inference->temperature = temperature;
    inference->top_k = top_k;
    inference->max_tokens = max_tokens;
    
    if (verbose) {
        printf("✓ Inference engine initialized\n\n");
    }
    
    // Generate text using proper forward pass
    generate_text_proper(inference, prompt, max_tokens, temperature, top_k, verbose);
    
    // Cleanup - skip to avoid double-free issues
    // In production, proper cleanup would be needed
    
    printf("\n");
    return 0;
}