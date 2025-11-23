/**
 * Activation Storage Implementation
 * 
 * Manages memory for storing forward pass activations
 * needed for backward pass gradient computation.
 */

#include "cllm_activations.h"
#include <string.h>
#include <stdio.h>

ForwardActivations* cllm_allocate_activations(
    int batch_size,
    int seq_len,
    uint32_t embed_dim,
    uint32_t vocab_size,
    uint32_t num_layers
) {
    ForwardActivations* act = (ForwardActivations*)calloc(1, sizeof(ForwardActivations));
    if (!act) return NULL;
    
    // Store dimensions
    act->batch_size = batch_size;
    act->seq_len = seq_len;
    act->embed_dim = embed_dim;
    act->vocab_size = vocab_size;
    act->num_layers = num_layers;
    
    size_t seq_size = batch_size * seq_len * embed_dim;
    size_t logits_size = batch_size * seq_len * vocab_size;
    
    // Allocate input embeddings
    act->input_embeddings = (float*)calloc(seq_size, sizeof(float));
    if (!act->input_embeddings) {
        cllm_free_activations(act);
        return NULL;
    }
    
    // Allocate per-layer arrays
    act->layer_inputs = (float**)calloc(num_layers, sizeof(float*));
    act->attention_outputs = (float**)calloc(num_layers, sizeof(float*));
    act->ff_outputs = (float**)calloc(num_layers, sizeof(float*));
    act->layer_outputs = (float**)calloc(num_layers, sizeof(float*));
    act->attention_scores = (float**)calloc(num_layers, sizeof(float*));
    act->ff_hidden = (float**)calloc(num_layers, sizeof(float*));
    
    if (!act->layer_inputs || !act->attention_outputs || !act->ff_outputs ||
        !act->layer_outputs || !act->attention_scores || !act->ff_hidden) {
        cllm_free_activations(act);
        return NULL;
    }
    
    // Allocate per-layer activations
    for (uint32_t i = 0; i < num_layers; i++) {
        act->layer_inputs[i] = (float*)calloc(seq_size, sizeof(float));
        act->attention_outputs[i] = (float*)calloc(seq_size, sizeof(float));
        act->ff_outputs[i] = (float*)calloc(seq_size, sizeof(float));
        act->layer_outputs[i] = (float*)calloc(seq_size, sizeof(float));
        act->attention_scores[i] = (float*)calloc(seq_size, sizeof(float));
        act->ff_hidden[i] = (float*)calloc(seq_size * 4, sizeof(float)); // Assume 4x expansion
        
        if (!act->layer_inputs[i] || !act->attention_outputs[i] || 
            !act->ff_outputs[i] || !act->layer_outputs[i] ||
            !act->attention_scores[i] || !act->ff_hidden[i]) {
            cllm_free_activations(act);
            return NULL;
        }
    }
    
    // Allocate final outputs
    act->final_hidden = (float*)calloc(seq_size, sizeof(float));
    act->logits = (float*)calloc(logits_size, sizeof(float));
    
    if (!act->final_hidden || !act->logits) {
        cllm_free_activations(act);
        return NULL;
    }
    
    return act;
}

void cllm_free_activations(ForwardActivations* activations) {
    if (!activations) return;
    
    // Free input embeddings
    free(activations->input_embeddings);
    
    // Free per-layer activations
    if (activations->layer_inputs) {
        for (uint32_t i = 0; i < activations->num_layers; i++) {
            free(activations->layer_inputs[i]);
        }
        free(activations->layer_inputs);
    }
    
    if (activations->attention_outputs) {
        for (uint32_t i = 0; i < activations->num_layers; i++) {
            free(activations->attention_outputs[i]);
        }
        free(activations->attention_outputs);
    }
    
    if (activations->ff_outputs) {
        for (uint32_t i = 0; i < activations->num_layers; i++) {
            free(activations->ff_outputs[i]);
        }
        free(activations->ff_outputs);
    }
    
    if (activations->layer_outputs) {
        for (uint32_t i = 0; i < activations->num_layers; i++) {
            free(activations->layer_outputs[i]);
        }
        free(activations->layer_outputs);
    }
    
    if (activations->attention_scores) {
        for (uint32_t i = 0; i < activations->num_layers; i++) {
            free(activations->attention_scores[i]);
        }
        free(activations->attention_scores);
    }
    
    if (activations->ff_hidden) {
        for (uint32_t i = 0; i < activations->num_layers; i++) {
            free(activations->ff_hidden[i]);
        }
        free(activations->ff_hidden);
    }
    
    // Free final outputs
    free(activations->final_hidden);
    free(activations->logits);
    
    // Free structure
    free(activations);
}

void cllm_zero_activations(ForwardActivations* activations) {
    if (!activations) return;
    
    size_t seq_size = activations->batch_size * activations->seq_len * activations->embed_dim;
    size_t logits_size = activations->batch_size * activations->seq_len * activations->vocab_size;
    
    // Zero input embeddings
    if (activations->input_embeddings) {
        memset(activations->input_embeddings, 0, seq_size * sizeof(float));
    }
    
    // Zero per-layer activations
    for (uint32_t i = 0; i < activations->num_layers; i++) {
        if (activations->layer_inputs[i]) {
            memset(activations->layer_inputs[i], 0, seq_size * sizeof(float));
        }
        if (activations->attention_outputs[i]) {
            memset(activations->attention_outputs[i], 0, seq_size * sizeof(float));
        }
        if (activations->ff_outputs[i]) {
            memset(activations->ff_outputs[i], 0, seq_size * sizeof(float));
        }
        if (activations->layer_outputs[i]) {
            memset(activations->layer_outputs[i], 0, seq_size * sizeof(float));
        }
        if (activations->attention_scores[i]) {
            memset(activations->attention_scores[i], 0, seq_size * sizeof(float));
        }
        if (activations->ff_hidden[i]) {
            memset(activations->ff_hidden[i], 0, seq_size * 4 * sizeof(float));
        }
    }
    
    // Zero final outputs
    if (activations->final_hidden) {
        memset(activations->final_hidden, 0, seq_size * sizeof(float));
    }
    if (activations->logits) {
        memset(activations->logits, 0, logits_size * sizeof(float));
    }
}