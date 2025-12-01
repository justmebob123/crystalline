// BigFixed helper macros
#define BF_CREATE_INIT(var, prec) BigFixed var; var = *big_fixed_create(prec)
#define BF_FREE(var) big_fixed_free(&var)

#include "loss_functions.h"
#include "bigfixed_core.h"
#include "prime_bigint_transcendental.h"
#include <stdlib.h>
#include <string.h>

/*
 * BigFixed Loss Functions
 * Arbitrary precision loss computation for training
 */

void cross_entropy_loss_bigfixed(
    BigFixed** logits,
    uint32_t* targets,
    BigFixed* loss,
    int batch_size,
    int num_classes,
    int precision
) {
    if (!logits || !targets || !loss) return;
    
    BigFixed sum;
    BigFixed sum = *big_fixed_create(precision);
    big_fixed_from_int(&sum, 0);
    
    for (int i = 0; i < batch_size; i++) {
        uint32_t target = targets[i];
        if (target >= (uint32_t)num_classes) continue;
        
        BigFixed** logit_row = &logits[i * num_classes];
        
        // Apply softmax and compute log probability
        BigFixed max_logit;
        BigFixed max_logit = *big_fixed_create(precision);
        big_fixed_assign(&max_logit, logit_row[0]);
        
        for (int j = 1; j < num_classes; j++) {
            if (big_fixed_cmp(logit_row[j], &max_logit) > 0) {
                big_fixed_assign(&max_logit, logit_row[j]);
            }
        }
        
        BigFixed exp_sum;
        BigFixed exp_sum = *big_fixed_create(precision);
        big_fixed_from_int(&exp_sum, 0);
        
        for (int j = 0; j < num_classes; j++) {
            BigFixed diff, exp_val;
            BigFixed diff = *big_fixed_create(precision);
            BigFixed exp_val = *big_fixed_create(precision);
            
            big_fixed_sub(&diff, logit_row[j], &max_logit);
            big_exp(&exp_val, &diff, precision);
            big_fixed_add(&exp_sum, &exp_sum, &exp_val);
            
            big_fixed_free(&diff);
            big_fixed_free(&exp_val);
        }
        
        // log_prob = logit[target] - max - log(sum)
        BigFixed log_sum, target_logit_norm, log_prob, neg_log_prob;
        BigFixed log_sum = *big_fixed_create(precision);
        BigFixed target_logit_norm = *big_fixed_create(precision);
        BigFixed log_prob = *big_fixed_create(precision);
        BigFixed neg_log_prob = *big_fixed_create(precision);
        
        big_ln(&log_sum, &exp_sum, precision);
        big_fixed_sub(&target_logit_norm, logit_row[target], &max_logit);
        big_fixed_sub(&log_prob, &target_logit_norm, &log_sum);
        big_fixed_neg(&neg_log_prob, &log_prob);
        
        big_fixed_add(&sum, &sum, &neg_log_prob);
        
        big_fixed_free(&max_logit);
        big_fixed_free(&exp_sum);
        big_fixed_free(&log_sum);
        big_fixed_free(&target_logit_norm);
        big_fixed_free(&log_prob);
        big_fixed_free(&neg_log_prob);
    }
    
    // Average loss
    BigFixed batch_size_fixed;
    BigFixed batch_size_fixed = *big_fixed_create(precision);
    big_fixed_from_int(&batch_size_fixed, batch_size);
    big_fixed_div(loss, &sum, &batch_size_fixed);
    
    big_fixed_free(&sum);
    big_fixed_free(&batch_size_fixed);
}

void softmax_bigfixed(
    BigFixed** logits,
    int size,
    int precision
) {
    if (!logits || size <= 0) return;
    
    // Find max for numerical stability
    BigFixed max_logit;
    BigFixed max_logit = *big_fixed_create(precision);
    big_fixed_assign(&max_logit, logits[0]);
    
    for (int i = 1; i < size; i++) {
        if (big_fixed_cmp(logits[i], &max_logit) > 0) {
            big_fixed_assign(&max_logit, logits[i]);
        }
    }
    
    // Compute exp and sum
    BigFixed sum;
    BigFixed sum = *big_fixed_create(precision);
    big_fixed_from_int(&sum, 0);
    
    for (int i = 0; i < size; i++) {
        BigFixed diff, exp_val;
        BigFixed diff = *big_fixed_create(precision);
        BigFixed exp_val = *big_fixed_create(precision);
        
        big_fixed_sub(&diff, logits[i], &max_logit);
        big_exp(&exp_val, &diff, precision);
        big_fixed_assign(logits[i], &exp_val);
        big_fixed_add(&sum, &sum, &exp_val);
        
        big_fixed_free(&diff);
        big_fixed_free(&exp_val);
    }
    
    // Normalize
    for (int i = 0; i < size; i++) {
        big_fixed_div(logits[i], logits[i], &sum);
    }
    
    big_fixed_free(&max_logit);
    big_fixed_free(&sum);
}

void log_softmax_bigfixed(
    BigFixed** logits,
    int size,
    int precision
) {
    if (!logits || size <= 0) return;
    
    // Find max
    BigFixed max_logit;
    BigFixed max_logit = *big_fixed_create(precision);
    big_fixed_assign(&max_logit, logits[0]);
    
    for (int i = 1; i < size; i++) {
        if (big_fixed_cmp(logits[i], &max_logit) > 0) {
            big_fixed_assign(&max_logit, logits[i]);
        }
    }
    
    // Compute log(sum(exp))
    BigFixed sum;
    BigFixed sum = *big_fixed_create(precision);
    big_fixed_from_int(&sum, 0);
    
    for (int i = 0; i < size; i++) {
        BigFixed diff, exp_val;
        BigFixed diff = *big_fixed_create(precision);
        BigFixed exp_val = *big_fixed_create(precision);
        
        big_fixed_sub(&diff, logits[i], &max_logit);
        big_exp(&exp_val, &diff, precision);
        big_fixed_add(&sum, &sum, &exp_val);
        
        big_fixed_free(&diff);
        big_fixed_free(&exp_val);
    }
    
    BigFixed log_sum;
    BigFixed log_sum = *big_fixed_create(precision);
    big_ln(&log_sum, &sum, precision);
    
    // log_softmax[i] = logit[i] - max - log(sum)
    for (int i = 0; i < size; i++) {
        BigFixed temp;
        BigFixed temp = *big_fixed_create(precision);
        big_fixed_sub(&temp, logits[i], &max_logit);
        big_fixed_sub(logits[i], &temp, &log_sum);
        big_fixed_free(&temp);
    }
    
    big_fixed_free(&max_logit);
    big_fixed_free(&sum);
    big_fixed_free(&log_sum);
}
