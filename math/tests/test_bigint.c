/**
 * @file test_bigint.c
 * @brief Test suite for BigInt operations
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 */

#include "math/bigint.h"
#include "math/arithmetic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name, condition) do { \
    if (condition) { \
        tests_passed++; \
        printf("✓ %s\n", name); \
    } else { \
        tests_failed++; \
        printf("✗ %s\n", name); \
    } \
} while(0)

void test_creation_and_conversion(void) {
    printf("\n=== Testing Creation and Conversion ===\n");
    
    BigInt* zero = bigint_new();
    TEST("Create zero", zero != NULL && bigint_is_zero(zero));
    bigint_free(zero);
    
    BigInt* from_int = bigint_from_int64(12345);
    TEST("From int64: 12345", from_int != NULL && bigint_to_int64(from_int) == 12345);
    bigint_free(from_int);
    
    BigInt* from_neg = bigint_from_int64(-9876);
    TEST("From int64: -9876", from_neg != NULL && bigint_to_int64(from_neg) == -9876);
    TEST("Is negative", bigint_is_negative(from_neg));
    bigint_free(from_neg);
    
    BigInt* from_uint = bigint_from_uint64(0xFFFFFFFF);
    TEST("From uint64: 0xFFFFFFFF", from_uint != NULL && bigint_to_uint64(from_uint) == 0xFFFFFFFF);
    bigint_free(from_uint);
    
    BigInt* from_hex = bigint_from_hex("1a2b3c");
    TEST("From hex: 1a2b3c", from_hex != NULL);
    char* hex_str = bigint_to_hex(from_hex);
    TEST("To hex matches", hex_str != NULL && strcmp(hex_str, "1a2b3c") == 0);
    free(hex_str);
    bigint_free(from_hex);
}

void test_comparison(void) {
    printf("\n=== Testing Comparison ===\n");
    
    BigInt* a = bigint_from_int64(100);
    BigInt* b = bigint_from_int64(200);
    BigInt* c = bigint_from_int64(100);
    BigInt* d = bigint_from_int64(-50);
    
    TEST("100 < 200", bigint_cmp(a, b) < 0);
    TEST("200 > 100", bigint_cmp(b, a) > 0);
    TEST("100 == 100", bigint_cmp(a, c) == 0);
    TEST("100 > -50", bigint_cmp(a, d) > 0);
    TEST("-50 < 100", bigint_cmp(d, a) < 0);
    
    TEST("-50 is negative", bigint_is_negative(d));
    
    BigInt* zero = bigint_new();
    TEST("0 is zero", bigint_is_zero(zero));
    TEST("0 is not negative", !bigint_is_negative(zero));
    
    bigint_free(a);
    bigint_free(b);
    bigint_free(c);
    bigint_free(d);
    bigint_free(zero);
}

void test_addition(void) {
    printf("\n=== Testing Addition ===\n");
    
    BigInt* a = bigint_from_int64(100);
    BigInt* b = bigint_from_int64(200);
    BigInt* result = bigint_new();
    
    bigint_add(result, a, b);
    TEST("100 + 200 = 300", bigint_to_int64(result) == 300);
    
    BigInt* c = bigint_from_int64(-50);
    bigint_add(result, a, c);
    TEST("100 + (-50) = 50", bigint_to_int64(result) == 50);
    
    bigint_add(result, c, c);
    TEST("(-50) + (-50) = -100", bigint_to_int64(result) == -100);
    
    BigInt* zero = bigint_new();
    bigint_add(result, a, zero);
    TEST("100 + 0 = 100", bigint_to_int64(result) == 100);
    
    bigint_free(a);
    bigint_free(b);
    bigint_free(c);
    bigint_free(zero);
    bigint_free(result);
}

void test_subtraction(void) {
    printf("\n=== Testing Subtraction ===\n");
    
    BigInt* a = bigint_from_int64(300);
    BigInt* b = bigint_from_int64(100);
    BigInt* result = bigint_new();
    
    bigint_sub(result, a, b);
    TEST("300 - 100 = 200", bigint_to_int64(result) == 200);
    
    bigint_sub(result, b, a);
    TEST("100 - 300 = -200", bigint_to_int64(result) == -200);
    
    BigInt* c = bigint_from_int64(-50);
    bigint_sub(result, a, c);
    TEST("300 - (-50) = 350", bigint_to_int64(result) == 350);
    
    bigint_sub(result, c, a);
    TEST("(-50) - 300 = -350", bigint_to_int64(result) == -350);
    
    bigint_free(a);
    bigint_free(b);
    bigint_free(c);
    bigint_free(result);
}

void test_multiplication(void) {
    printf("\n=== Testing Multiplication ===\n");
    
    BigInt* a = bigint_from_int64(12);
    BigInt* b = bigint_from_int64(34);
    BigInt* result = bigint_new();
    
    bigint_mul(result, a, b);
    TEST("12 * 34 = 408", bigint_to_int64(result) == 408);
    
    BigInt* c = bigint_from_int64(-5);
    bigint_mul(result, a, c);
    TEST("12 * (-5) = -60", bigint_to_int64(result) == -60);
    
    bigint_mul(result, c, c);
    TEST("(-5) * (-5) = 25", bigint_to_int64(result) == 25);
    
    BigInt* zero = bigint_new();
    bigint_mul(result, a, zero);
    TEST("12 * 0 = 0", bigint_is_zero(result));
    
    bigint_free(a);
    bigint_free(b);
    bigint_free(c);
    bigint_free(zero);
    bigint_free(result);
}

void test_division(void) {
    printf("\n=== Testing Division ===\n");
    
    BigInt* a = bigint_from_int64(100);
    BigInt* b = bigint_from_int64(10);
    BigInt* quotient = bigint_new();
    BigInt* remainder = bigint_new();
    
    bigint_div(quotient, remainder, a, b);
    TEST("100 / 10 = 10", bigint_to_int64(quotient) == 10);
    TEST("100 % 10 = 0", bigint_is_zero(remainder));
    
    BigInt* c = bigint_from_int64(7);
    bigint_div(quotient, remainder, a, c);
    TEST("100 / 7 = 14", bigint_to_int64(quotient) == 14);
    TEST("100 % 7 = 2", bigint_to_int64(remainder) == 2);
    
    BigInt* d = bigint_from_int64(200);
    bigint_div(quotient, remainder, a, d);
    TEST("100 / 200 = 0", bigint_is_zero(quotient));
    TEST("100 % 200 = 100", bigint_to_int64(remainder) == 100);
    
    bigint_free(a);
    bigint_free(b);
    bigint_free(c);
    bigint_free(d);
    bigint_free(quotient);
    bigint_free(remainder);
}

void test_large_numbers(void) {
    printf("\n=== Testing Large Numbers ===\n");
    
    BigInt* large1 = bigint_from_hex("ffffffff");  // 2^32 - 1
    BigInt* large2 = bigint_from_hex("ffffffff");
    BigInt* result = bigint_new();
    
    bigint_add(result, large1, large2);
    char* hex_result = bigint_to_hex(result);
    TEST("0xFFFFFFFF + 0xFFFFFFFF = 0x1FFFFFFFE", 
         hex_result != NULL && strcmp(hex_result, "1fffffffe") == 0);
    free(hex_result);
    
    bigint_mul(result, large1, large2);
    hex_result = bigint_to_hex(result);
    TEST("0xFFFFFFFF * 0xFFFFFFFF (large multiplication)", hex_result != NULL);
    free(hex_result);
    
    bigint_free(large1);
    bigint_free(large2);
    bigint_free(result);
}

void test_copy(void) {
    printf("\n=== Testing Copy ===\n");
    
    BigInt* original = bigint_from_int64(12345);
    BigInt* copy = bigint_copy(original);
    
    TEST("Copy created", copy != NULL);
    TEST("Copy equals original", bigint_cmp(original, copy) == 0);
    TEST("Copy is different object", original != copy);
    
    bigint_free(original);
    bigint_free(copy);
}

int main(void) {
    printf("Crystalline Math Library - BigInt Tests\n");
    printf("========================================\n");
    
    test_creation_and_conversion();
    test_comparison();
    test_addition();
    test_subtraction();
    test_multiplication();
    test_division();
    test_large_numbers();
    test_copy();
    
    printf("\n========================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);
    
    if (tests_failed > 0) {
        printf("FAILED: Some tests did not pass\n");
        return 1;
    }
    
    printf("SUCCESS: All tests passed!\n");
    return 0;
}