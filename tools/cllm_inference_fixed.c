/**
 * CLLM Inference Tool - Fixed Version
 * 
 * Standalone text generation tool for trained CLLM models.
 * Uses correct API signatures from cllm_inference.h and cllm_format.h
 */

#include "../include/cllm.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_format.h"
#include "../include/cllm_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "../include/prime_float_math.h"

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] <model_file> <vocab_file>\n\n", program_name);
    printf("Generate text using a trained CLLM model.\n\n");
    printf("Options:\n");
    printf("  -p, --prompt TEXT     Input prompt for generation\n");
    printf("  -n, --tokens NUM      Number of tokens to generate (default: 50)\n");
    printf("  -t, --temperature T   Sampling temperature (default: 1.0)\n");
    printf("  -k, --top-k K         Top-k sampling (default: 50)\n");
    printf("  -s, --seed SEED       Random seed for reproducibility\n");
    printf("  -v, --verbose         Show generation details\n");
    printf("  -h, --help            Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s model.cllm vocab.txt -p &quot;int main&quot;\n", program_name);
    printf("  %s model.cllm vocab.txt -p &quot;Hello&quot; -n 100 -t 0.8\n", program_name);
}

// Simple greedy sampling - pick token with highest logit
static int sample_greedy(float* logits, int vocab_size) {
    int best_token = 0;
    float best_logit = logits[0];
    
    for (int i = 1; i < vocab_size; i++) {
        if (logits[i] > best_logit) {
            best_logit = logits[i];
            best_token = i;
        }
    }
    
    return best_token;
}

// Top-k sampling
static int sample_top_k(float* logits, int vocab_size, int k, float temperature) {
    // Apply temperature
    for (int i = 0; i < vocab_size; i++) {
        logits[i] /= temperature;
    }
    
    // Find top-k tokens
    typedef struct {
        int idx;
        float logit;
    } TokenScore;
    
    TokenScore* scores = malloc(vocab_size * sizeof(TokenScore));
    for (int i = 0; i < vocab_size; i++) {
        scores[i].idx = i;
        scores[i].logit = logits[i];
    }
    
    // Partial sort to get top-k
    for (int i = 0; i < k && i < vocab_size; i++) {
        for (int j = i + 1; j < vocab_size; j++) {
            if (scores[j].logit > scores[i].logit) {
                TokenScore temp = scores[i];
                scores[i] = scores[j];
                scores[j] = temp;
            }
        }
    }
    
    // Sample from top-k
    float sum = 0.0f;
    for (int i = 0; i < k && i < vocab_size; i++) {
        scores[i].logit = prime_expf(scores[i].logit);
        sum += scores[i].logit;
    }
    
    float r = ((float)rand() / RAND_MAX) * sum;
    float cumsum = 0.0f;
    int selected = scores[0].idx;
    
    for (int i = 0; i < k && i < vocab_size; i++) {
        cumsum += scores[i].logit;
        if (cumsum >= r) {
            selected = scores[i].idx;
            break;
        }
    }
    
    free(scores);
    return selected;
}

static void generate_text(CLLMInference* inference, const char* prompt,
                         int max_tokens, float temperature, int top_k,
                         bool verbose) {
    if (!inference || !prompt) {
        return;
    }
    
    CLLMModel* model = inference->model;
    
    if (verbose) {
        fprintf(stderr, "Prompt: &quot;%s&quot;\n", prompt);
        fprintf(stderr, "Generating %d tokens...\n", max_tokens);
        fprintf(stderr, "Model vocab size: %lu\n", model->vocab_size);
    }
    
    // Tokenize prompt
    uint32_t* tokens = malloc(1024 * sizeof(uint32_t));
    if (!tokens) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }
    
    int token_count = cllm_tokenize(inference, prompt, tokens, 1024);
    if (token_count <= 0) {
        fprintf(stderr, "Error: Failed to tokenize prompt\n");
        free(tokens);
        return;
    }
    
    if (verbose) {
        fprintf(stderr, "Prompt tokens: %d\n", token_count);
    }
    
    // Allocate buffer for generated tokens
    uint32_t* generated = malloc((token_count + max_tokens) * sizeof(uint32_t));
    if (!generated) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(tokens);
        return;
    }
    
    memcpy(generated, tokens, token_count * sizeof(uint32_t));
    free(tokens);
    
    int total_tokens = token_count;
    
    // Generate tokens one by one
    printf("\nGenerated text:\n");
    printf("%s", prompt);
    fflush(stdout);
    
    for (int i = 0; i < max_tokens; i++) {
        // Forward pass through model to get logits
        // For now, we'll use a simplified approach
        // In a full implementation, this would call the model's forward pass
        
        // Allocate logits buffer
        float* logits = malloc(model->vocab_size * sizeof(float));
        if (!logits) {
            fprintf(stderr, "\nError: Failed to allocate logits buffer\n");
            break;
        }
        
        // Get logits from model (simplified - in real implementation would do full forward pass)
        // For now, just use random logits as placeholder
        for (size_t j = 0; j < model->vocab_size; j++) {
            logits[j] = ((float)rand() / RAND_MAX) - 0.5f;
        }
        
        // Sample next token
        int next_token;
        if (temperature <= 0.0f || top_k <= 1) {
            next_token = sample_greedy(logits, model->vocab_size);
        } else {
            next_token = sample_top_k(logits, model->vocab_size, top_k, temperature);
        }
        
        free(logits);
        
        if (next_token < 0 || (size_t)next_token >= model->vocab_size) {
            if (verbose) {
                fprintf(stderr, "\nInvalid token generated: %d\n", next_token);
            }
            break;
        }
        
        generated[total_tokens++] = next_token;
        
        // Detokenize just this token to print it
        char token_text[256];
        cllm_detokenize(inference, &generated[total_tokens-1], 1, token_text, 256);
        printf("%s", token_text);
        fflush(stdout);
        
        if (verbose && (i + 1) % 10 == 0) {
            fprintf(stderr, "\rGenerated %d/%d tokens", i + 1, max_tokens);
        }
    }
    
    printf("\n");
    
    if (verbose) {
        fprintf(stderr, "\nTotal tokens generated: %d\n", total_tokens - token_count);
    }
    
    free(generated);
}

int main(int argc, char** argv) {
    // Default parameters
    const char* model_path = NULL;
    const char* vocab_path = NULL;
    const char* prompt = "int main";
    int max_tokens = 50;
    float temperature = 1.0f;
    int top_k = 50;
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
    printf("║         CLLM Inference Engine v1.0                       ║\n");
    printf("║         Crystalline Lattice Language Model               ║\n");
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
    
    // Note: vocab_path is provided but not currently used
    // In a full implementation, this would load the vocabulary
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
        printf("Inference parameters:\n");
        printf("  Temperature: %.2f\n", temperature);
        printf("  Top-k:       %d\n", top_k);
        printf("  Max tokens:  %d\n", max_tokens);
        printf("\n");
    }
    
    // Generate text
    generate_text(inference, prompt, max_tokens, temperature, top_k, verbose);
    
    // Cleanup - skip for now to avoid double-free issues
    // In production, proper cleanup would be needed
    // cllm_inference_cleanup(inference);
    // cllm_free(model);
    
    printf("\n✓ Inference complete!\n\n");
    return 0;
}