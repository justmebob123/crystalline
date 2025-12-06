#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cllm.h"
#include "../include/cllm_training.h"
#include "../include/cllm_inference.h"
#include "../include/cllm_utils.h"
#include "../include/cllm_format.h"
#include "../include/cllm_tokenizer.h"

int main() {
    printf("=== FINAL PIPELINE TEST ===\n\n");
    
    // Create model
    printf("Creating model...\n");
    CLLMModel* model = cllm_create_small_model();
    if (!model) return 1;
    printf("Model created\n\n");
    
    // Load and tokenize text
    printf("Loading text...\n");
    CLLMTokenizer* tokenizer = cllm_create_tokenizer(model->vocab_size);
    if (!tokenizer) {
        cllm_free_model(model);
        return 1;
    }
    
    FILE* f = fopen("data/simple_test.txt", "r");
    if (!f) {
        fprintf(stderr, "Failed to open file\n");
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    char text[1024];
    size_t bytes = fread(text, 1, sizeof(text)-1, f);
    text[bytes] = '\0';
    fclose(f);
    
    printf("Text: %s\n", text);
    
    // Build vocab and tokenize
    cllm_build_vocab(tokenizer, text);
    printf("Vocabulary: %u tokens\n", tokenizer->vocab_size);
    
    uint32_t num_tokens = 0;
    uint32_t* tokens = cllm_tokenizer_encode(tokenizer, text, &num_tokens);
    if (!tokens) {
        fprintf(stderr, "Tokenization failed\n");
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    printf("Tokenized: %u tokens\n\n", num_tokens);
    
    // Copy vocabulary to model
    if (tokenizer->vocab) {
        if (model->tokens) free(model->tokens);
        model->tokens = (CLLMToken*)calloc(model->vocab_size, sizeof(CLLMToken));
        if (model->tokens) {
            for (uint32_t i = 0; i < tokenizer->vocab_size && i < model->vocab_size; i++) {
                if (tokenizer->vocab[i]) {
                    strncpy(model->tokens[i].token_str, tokenizer->vocab[i], 63);
                    model->tokens[i].token_str[63] = '\0';
                    model->tokens[i].token_id = i;
                }
            }
        }
    }
    
    // Train
    printf("Training (50 steps, lr=0.0001)...\n");
    CLLMTrainingConfig config = {
        .num_epochs = 1,
        .batch_size = 1,
        .sequence_length = 4,
        .learning_rate = 0.0001,  // Much lower learning rate
        .gradient_accumulation_steps = 1,
        .save_every = 1000,
        .use_mixed_precision = false
    };
    
    CLLMTraining* training = cllm_training_init(model, &config);
    if (!training) {
        fprintf(stderr, "Training init failed\n");
        free(tokens);
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    training->tokens = tokens;
    training->num_tokens = num_tokens;
    
    uint32_t batch[4], target[4];
    float losses[50];
    
    for (int step = 0; step < 50; step++) {
        int start = (step * 4) % num_tokens;
        for (int i = 0; i < 4; i++) {
            batch[i] = tokens[(start + i) % num_tokens];
            target[i] = tokens[(start + i + 1) % num_tokens];
        }
        
        cllm_forward_training(training, batch);
        losses[step] = cllm_compute_loss(training, batch, target, 4);
        cllm_backward_training(training, target, NULL);
        cllm_optimizer_step(training);
        
        if (step % 10 == 0) {
            printf("  Step %2d: Loss = %.4f\n", step, losses[step]);
        }
    }
    
    printf("\nLoss: %.4f -> %.4f (%.1f%%)\n\n", 
           losses[0], losses[49], 100.0 * (losses[49] - losses[0]) / losses[0]);
    
    // Test inference
    printf("Testing inference...\n");
    CLLMInference* inference = cllm_inference_init(model);
    if (!inference) {
        fprintf(stderr, "Inference init failed\n");
        cllm_training_cleanup(training);
        cllm_free_tokenizer(tokenizer);
        cllm_free_model(model);
        return 1;
    }
    
    cllm_set_temperature(inference, 0.5);
    cllm_set_max_tokens(inference, 10);
    
    char output[1024];
    
    printf("\nTest 1: &quot;The quick&quot;\n");
    int gen1 = cllm_generate(inference, "The quick", output, sizeof(output));
    printf("Output: %s\n", output);
    
    printf("\nTest 2: &quot;brown fox&quot;\n");
    int gen2 = cllm_generate(inference, "brown fox", output, sizeof(output));
    printf("Output: %s\n", output);
    
    printf("\nTest 3: &quot;test&quot;\n");
    int gen3 = cllm_generate(inference, "test", output, sizeof(output));
    printf("Output: %s\n", output);
    
    printf("\n=== COMPLETE ===\n");
    printf("Generated tokens: %d, %d, %d\n", gen1, gen2, gen3);
    
    // Cleanup
    cllm_inference_cleanup(inference);
    cllm_training_cleanup(training);
    cllm_free_tokenizer(tokenizer);
    cllm_free_model(model);
    
    return 0;
}