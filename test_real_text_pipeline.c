#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_utils.h"
#include "../include/cllm_format.h"
#include "../include/cllm_tokenizer.h"
#include "../include/cllm_data_loader.h"

int main() {
    printf("=== REAL TEXT PIPELINE TEST ===\n\n");
    
    // Step 1: Create model
    printf("Step 1: Creating model...\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) {
        fprintf(stderr, "Failed to create model\n");
        return 1;
    }
    printf("Model created\n\n");
    
    // Step 2: Create tokenizer and load real text
    printf("Step 2: Loading text data...\n");
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(model->vocab_size);
    if (!tokenizer) {
        fprintf(stderr, "Failed to create tokenizer\n");
        cllm_free_model(model);
        return 1;
    }
    
    // Read the text file
    FILE* f = fopen("data/simple_test.txt", "r");
    if (!f) {
        fprintf(stderr, "Failed to open data/simple_test.txt\n");
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    char text[1024];
    size_t bytes_read = fread(text, 1, sizeof(text)-1, f);
    text[bytes_read] = '\0';
    fclose(f);
    
    printf("Loaded text: %s\n", text);
    
    // Build vocabulary from text
    printf("Building vocabulary...\n");
    cllm_build_vocab(tokenizer, text);
    printf("Vocabulary size: %u\n\n", tokenizer->vocab_size);
    
    // Tokenize the text
    uint32_t num_tokens_encoded = 0;
    uint32_t* tokens = cllm_tokenizer_encode(tokenizer, text, &num_tokens_encoded);
    if (!tokens || num_tokens_encoded == 0) {
        fprintf(stderr, "Failed to tokenize text\n");
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    printf("Tokenized into %u tokens\n\n", num_tokens_encoded);
    
    // Step 3: Train
    printf("Step 3: Training (10 steps)...\n");
    CLLMTrainingConfig config = {
        .num_epochs = 1,
        .batch_size = 1,
        .sequence_length = 4,
        .learning_rate = 0.001,  // Lower learning rate
        .gradient_accumulation_steps = 1,
        .save_every = 1000,
        .use_mixed_precision = false
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Failed to initialize training\n");
        free(tokens);
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    training->tokens = tokens;
    training->num_tokens = num_tokens_encoded;
    
    // Train for 10 steps
    uint32_t batch[4], target[4];
    float losses[10];
    
    for (int step = 0; step < 10; step++) {
        int start = (step * 4) % num_tokens_encoded;
        for (int i = 0; i < 4; i++) {
            batch[i] = tokens[(start + i) % num_tokens_encoded];
            target[i] = tokens[(start + i + 1) % num_tokens_encoded];
        }
        
        cllm_forward_training(training, batch);
        losses[step] = cllm_compute_loss(training, batch, target, 4);
        cllm_backward_training(training, target, NULL);
        cllm_optimizer_step(training);
        
        printf("  Step %d: Loss = %.4f\n", step, losses[step]);
    }
    
    printf("\nLoss: %.4f -> %.4f (change: %.1f%%)\n\n", 
           losses[0], losses[9], 100.0 * (losses[9] - losses[0]) / losses[0]);
    
    // Step 4: Test inference
    printf("Step 4: Testing inference...\n");
    
    // Copy tokenizer vocabulary to model for detokenization
    // This is necessary for inference to work
    printf("Model tokens before copy: %p\n", (void*)model->tokens);
    printf("Tokenizer vocab: %p\n", (void*)tokenizer->vocab);
    
    if (tokenizer->vocab) {
        // Always allocate fresh tokens array
        if (model->tokens) {
            free(model->tokens);
        }
        model->tokens = (CLLMToken*)calloc(model->vocab_size, sizeof(CLLMToken));
        if (model->tokens) {
            int copied = 0;
            for (uint32_t i = 0; i < tokenizer->vocab_size && i < model->vocab_size; i++) {
                if (tokenizer->vocab[i]) {
                    strncpy(model->tokens[i].token_str, tokenizer->vocab[i], 63);
                    model->tokens[i].token_str[63] = '\0';
                    model->tokens[i].token_id = i;
                    copied++;
                }
            }
            printf("Copied vocabulary to model: %d/%u tokens\n", copied, tokenizer->vocab_size);
            
            // Debug: print first few tokens
            printf("Sample tokens: ");
            for (int i = 0; i < 5 && i < tokenizer->vocab_size; i++) {
                if (tokenizer->vocab[i]) {
                    printf("[%d:'%s'] ", i, tokenizer->vocab[i]);
                }
            }
            printf("\n");
        }
    } else {
        printf("No vocabulary to copy!\n");
    }
    
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Failed to initialize inference\n");
        cllm_training_cleanup(training);
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    cllm_set_temperature(inference, 0.8);
    cllm_set_max_tokens(inference, 20);
    
    char output[1024];
    printf("Prompt: &quot;The quick&quot;\n");
    int gen = cllm_generate(inference, "The quick", output, sizeof(output));
    printf("Generated: %s\n", output);
    printf("Tokens: %d\n\n", gen);
    
    printf("=== TEST COMPLETE ===\n");
    
    // Cleanup
    cllm_inference_cleanup(inference);
    cllm_training_cleanup(training);
    cllm_free_tokenizer(tokenizer);
    cllm_free_model(model);
    
    return 0;
}