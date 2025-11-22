#include "include/cllm_pure_crystalline.h"
#include <stdio.h>

int main() {
    printf("Creating token...\n");
    CrystallineToken* token = crystalline_token_create(0, "test", 5);
    
    if (token) {
        printf("Token created successfully!\n");
        printf("  ID: %u\n", token->token_id);
        printf("  String: %s\n", token->token_str);
        printf("  Prime: %lu\n", token->prime);
        printf("  Is root: %s\n", token->is_root ? "YES" : "NO");
        
        crystalline_token_free(token);
        printf("Token freed successfully!\n");
    } else {
        printf("Failed to create token\n");
    }
    
    return 0;
}
