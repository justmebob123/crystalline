#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bigint_core.h"

int main() {
    BigInt a, b, result;
    big_init(&a);
    big_init(&b);
    big_init(&result);
    
    // Test addition
    big_from_int(&a, 12345);
    big_from_int(&b, 67890);
    
    printf("a = ");
    char* str_a = big_to_string(&a);
    printf("%s\n", str_a);
    free(str_a);
    
    printf("b = ");
    char* str_b = big_to_string(&b);
    printf("%s\n", str_b);
    free(str_b);
    
    big_add(&a, &b, &result);
    
    printf("a + b = ");
    char* str_result = big_to_string(&result);
    printf("%s\n", str_result);
    printf("Expected: 80235\n");
    printf("Match: %s\n", strcmp(str_result, "80235") == 0 ? "YES" : "NO");
    free(str_result);
    
    big_free(&a);
    big_free(&b);
    big_free(&result);
    
    return 0;
}