/*
 * test_ntt_debug.c - Debug NTT initialization
 */

#include <stdio.h>
#include <stdlib.h>
#include "include/bigint_core.h"
#include "include/bigint_ntt.h"

int main() {
    printf("Testing NTT initialization with n=8\n\n");
    
    NTTContext ctx;
    
    printf("Calling ntt_init(&ctx, 8)...\n");
    int result = ntt_init(&ctx, 8);
    
    if (result) {
        printf("✓ NTT initialization successful!\n\n");
        
        printf("Prime: ");
        char* p_str = big_to_string(&ctx.prime);
        printf("%s\n", p_str);
        free(p_str);
        
        printf("Root: ");
        char* r_str = big_to_string(&ctx.root);
        printf("%s\n", r_str);
        free(r_str);
        
        printf("Transform size: %zu\n", ctx.n);
        
        // Test forward roots
        printf("\nForward roots:\n");
        for (size_t i = 0; i < 8; i++) {
            char* root_str = big_to_string(&ctx.roots_forward[i]);
            printf("  ω^%zu = %s\n", i, root_str);
            free(root_str);
        }
        
        ntt_free(&ctx);
    } else {
        printf("✗ NTT initialization failed\n");
    }
    
    return result ? 0 : 1;
}
