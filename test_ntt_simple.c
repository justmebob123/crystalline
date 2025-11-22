#include <stdio.h>
#include "include/bigint_core.h"
#include "include/bigint_ntt.h"

int main() {
    printf("Simple NTT Forward/Inverse Test\n\n");
    
    NTTContext ctx;
    if (!ntt_init(&ctx, 8)) {
        printf("Failed to init NTT\n");
        return 1;
    }
    
    // Create simple input: [1, 0, 0, 0, 0, 0, 0, 0]
    BigInt input[8], transformed[8], recovered[8];
    
    for (int i = 0; i < 8; i++) {
        big_init(&input[i]);
        big_from_int(&input[i], (i == 0) ? 1 : 0);
    }
    
    printf("Input: ");
    for (int i = 0; i < 8; i++) {
        char* s = big_to_string(&input[i]);
        printf("%s ", s);
        free(s);
    }
    printf("\n");
    
    // Forward NTT
    ntt_forward(&ctx, transformed, input, 8);
    
    printf("After forward NTT: ");
    for (int i = 0; i < 8; i++) {
        char* s = big_to_string(&transformed[i]);
        printf("%s ", s);
        free(s);
    }
    printf("\n");
    
    // Inverse NTT
    ntt_inverse(&ctx, recovered, transformed, 8);
    
    printf("After inverse NTT: ");
    for (int i = 0; i < 8; i++) {
        char* s = big_to_string(&recovered[i]);
        printf("%s ", s);
        free(s);
    }
    printf("\n");
    
    // Check if recovered matches input
    int match = 1;
    for (int i = 0; i < 8; i++) {
        if (big_cmp(&recovered[i], &input[i]) != 0) {
            match = 0;
            break;
        }
    }
    
    printf("\nResult: %s\n", match ? "✓ PASS" : "✗ FAIL");
    
    // Cleanup
    for (int i = 0; i < 8; i++) {
        big_free(&input[i]);
        big_free(&transformed[i]);
        big_free(&recovered[i]);
    }
    ntt_free(&ctx);
    
    return match ? 0 : 1;
}
