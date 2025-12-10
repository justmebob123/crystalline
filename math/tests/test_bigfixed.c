/**
 * @file test_bigfixed.c
 * @brief Test suite for BigFixed operations
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 */

#include "math/bigfixed.h"
#include "math/arithmetic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_EPSILON 1e-6

#define TEST(name, condition) do { \
    if (condition) { \
        tests_passed++; \
        printf("✓ %s\n", name); \
    } else { \
        tests_failed++; \
        printf("✗ %s\n", name); \
    } \
} while(0)

#define TEST_APPROX(name, actual, expected) do { \
    double diff = math_abs((actual) - (expected)); \
    if (diff < TEST_EPSILON) { \
        tests_passed++; \
        printf("✓ %s (%.6f ≈ %.6f)\n", name, actual, expected); \
    } else { \
        tests_failed++; \
        printf("✗ %s (%.6f != %.6f, diff=%.6e)\n", name, actual, expected, diff); \
    } \
} while(0)

void test_creation_and_conversion(void) {
    printf("\n=== Testing Creation and Conversion ===\n");
    
    BigFixed* zero = bigfixed_new(64);
    TEST("Create zero", zero != NULL && bigfixed_is_zero(zero));
    bigfixed_free(zero);
    
    BigFixed* from_double = bigfixed_from_double(3.14159, 64);
    TEST("From double: 3.14159", from_double != NULL);
    double back = bigfixed_to_double(from_double);
    TEST_APPROX("To double matches", back, 3.14159);
    bigfixed_free(from_double);
    
    BigFixed* from_int = bigfixed_from_int64(42, 64);
    TEST("From int64: 42", from_int != NULL);
    TEST("To int64 matches", bigfixed_to_int64(from_int) == 42);
    bigfixed_free(from_int);
    
    BigFixed* from_neg = bigfixed_from_double(-2.5, 64);
    TEST("From double: -2.5", from_neg != NULL);
    TEST("Is negative", bigfixed_is_negative(from_neg));
    bigfixed_free(from_neg);
    
    BigFixed* from_str = bigfixed_from_string("1.23456", 64);
    TEST("From string: 1.23456", from_str != NULL);
    double str_val = bigfixed_to_double(from_str);
    TEST_APPROX("String conversion", str_val, 1.23456);
    bigfixed_free(from_str);
}

void test_comparison(void) {
    printf("\n=== Testing Comparison ===\n");
    
    BigFixed* a = bigfixed_from_double(1.5, 64);
    BigFixed* b = bigfixed_from_double(2.5, 64);
    BigFixed* c = bigfixed_from_double(1.5, 64);
    BigFixed* d = bigfixed_from_double(-1.0, 64);
    
    TEST("1.5 < 2.5", bigfixed_cmp(a, b) < 0);
    TEST("2.5 > 1.5", bigfixed_cmp(b, a) > 0);
    TEST("1.5 == 1.5", bigfixed_cmp(a, c) == 0);
    TEST("1.5 > -1.0", bigfixed_cmp(a, d) > 0);
    
    TEST("1.5 is positive", bigfixed_is_positive(a));
    TEST("-1.0 is negative", bigfixed_is_negative(d));
    
    BigFixed* zero = bigfixed_new(64);
    TEST("0 is zero", bigfixed_is_zero(zero));
    TEST("0 is not negative", !bigfixed_is_negative(zero));
    
    bigfixed_free(a);
    bigfixed_free(b);
    bigfixed_free(c);
    bigfixed_free(d);
    bigfixed_free(zero);
}

void test_addition(void) {
    printf("\n=== Testing Addition ===\n");
    
    BigFixed* a = bigfixed_from_double(1.5, 64);
    BigFixed* b = bigfixed_from_double(2.5, 64);
    BigFixed* result = bigfixed_new(64);
    
    bigfixed_add(result, a, b);
    TEST_APPROX("1.5 + 2.5 = 4.0", bigfixed_to_double(result), 4.0);
    
    BigFixed* c = bigfixed_from_double(-1.0, 64);
    bigfixed_add(result, a, c);
    TEST_APPROX("1.5 + (-1.0) = 0.5", bigfixed_to_double(result), 0.5);
    
    bigfixed_add(result, c, c);
    TEST_APPROX("(-1.0) + (-1.0) = -2.0", bigfixed_to_double(result), -2.0);
    
    BigFixed* zero = bigfixed_new(64);
    bigfixed_add(result, a, zero);
    TEST_APPROX("1.5 + 0 = 1.5", bigfixed_to_double(result), 1.5);
    
    bigfixed_free(a);
    bigfixed_free(b);
    bigfixed_free(c);
    bigfixed_free(zero);
    bigfixed_free(result);
}

void test_subtraction(void) {
    printf("\n=== Testing Subtraction ===\n");
    
    BigFixed* a = bigfixed_from_double(5.0, 64);
    BigFixed* b = bigfixed_from_double(2.0, 64);
    BigFixed* result = bigfixed_new(64);
    
    bigfixed_sub(result, a, b);
    TEST_APPROX("5.0 - 2.0 = 3.0", bigfixed_to_double(result), 3.0);
    
    bigfixed_sub(result, b, a);
    TEST_APPROX("2.0 - 5.0 = -3.0", bigfixed_to_double(result), -3.0);
    
    BigFixed* c = bigfixed_from_double(-1.5, 64);
    bigfixed_sub(result, a, c);
    TEST_APPROX("5.0 - (-1.5) = 6.5", bigfixed_to_double(result), 6.5);
    
    bigfixed_free(a);
    bigfixed_free(b);
    bigfixed_free(c);
    bigfixed_free(result);
}

void test_multiplication(void) {
    printf("\n=== Testing Multiplication ===\n");
    
    BigFixed* a = bigfixed_from_double(2.5, 64);
    BigFixed* b = bigfixed_from_double(4.0, 64);
    BigFixed* result = bigfixed_new(64);
    
    bigfixed_mul(result, a, b);
    TEST_APPROX("2.5 * 4.0 = 10.0", bigfixed_to_double(result), 10.0);
    
    BigFixed* c = bigfixed_from_double(-2.0, 64);
    bigfixed_mul(result, a, c);
    TEST_APPROX("2.5 * (-2.0) = -5.0", bigfixed_to_double(result), -5.0);
    
    bigfixed_mul(result, c, c);
    TEST_APPROX("(-2.0) * (-2.0) = 4.0", bigfixed_to_double(result), 4.0);
    
    BigFixed* zero = bigfixed_new(64);
    bigfixed_mul(result, a, zero);
    TEST_APPROX("2.5 * 0 = 0", bigfixed_to_double(result), 0.0);
    
    bigfixed_free(a);
    bigfixed_free(b);
    bigfixed_free(c);
    bigfixed_free(zero);
    bigfixed_free(result);
}

void test_division(void) {
    printf("\n=== Testing Division ===\n");
    
    BigFixed* a = bigfixed_from_double(10.0, 64);
    BigFixed* b = bigfixed_from_double(2.0, 64);
    BigFixed* result = bigfixed_new(64);
    
    bigfixed_div(result, a, b);
    TEST_APPROX("10.0 / 2.0 = 5.0", bigfixed_to_double(result), 5.0);
    
    BigFixed* c = bigfixed_from_double(3.0, 64);
    bigfixed_div(result, a, c);
    TEST_APPROX("10.0 / 3.0 = 3.333...", bigfixed_to_double(result), 3.333333);
    
    BigFixed* d = bigfixed_from_double(-2.0, 64);
    bigfixed_div(result, a, d);
    TEST_APPROX("10.0 / (-2.0) = -5.0", bigfixed_to_double(result), -5.0);
    
    bigfixed_free(a);
    bigfixed_free(b);
    bigfixed_free(c);
    bigfixed_free(d);
    bigfixed_free(result);
}

void test_transcendental(void) {
    printf("\n=== Testing Transcendental Functions ===\n");
    
    BigFixed* x = bigfixed_from_double(4.0, 64);
    BigFixed* result = bigfixed_new(64);
    
    bigfixed_sqrt(result, x);
    TEST_APPROX("sqrt(4.0) = 2.0", bigfixed_to_double(result), 2.0);
    
    BigFixed* base = bigfixed_from_double(2.0, 64);
    BigFixed* exp = bigfixed_from_double(3.0, 64);
    bigfixed_pow(result, base, exp);
    TEST_APPROX("pow(2.0, 3.0) = 8.0", bigfixed_to_double(result), 8.0);
    
    BigFixed* one = bigfixed_from_double(1.0, 64);
    bigfixed_exp(result, one);
    double exp_result = bigfixed_to_double(result);
    TEST("exp(1.0) = e (within tolerance)", math_abs(exp_result - 2.71828) < 1e-5);
    
    BigFixed* ten = bigfixed_from_double(10.0, 64);
    bigfixed_log(result, ten);
    TEST_APPROX("log(10.0) = 2.302...", bigfixed_to_double(result), 2.302585);
    
    BigFixed* zero = bigfixed_from_double(0.0, 64);
    bigfixed_sin(result, zero);
    TEST_APPROX("sin(0.0) = 0.0", bigfixed_to_double(result), 0.0);
    
    bigfixed_cos(result, zero);
    TEST_APPROX("cos(0.0) = 1.0", bigfixed_to_double(result), 1.0);
    
    bigfixed_free(x);
    bigfixed_free(base);
    bigfixed_free(exp);
    bigfixed_free(one);
    bigfixed_free(ten);
    bigfixed_free(zero);
    bigfixed_free(result);
}

void test_negation_and_abs(void) {
    printf("\n=== Testing Negation and Absolute Value ===\n");
    
    BigFixed* a = bigfixed_from_double(5.5, 64);
    BigFixed* result = bigfixed_new(64);
    
    bigfixed_neg(result, a);
    TEST_APPROX("neg(5.5) = -5.5", bigfixed_to_double(result), -5.5);
    
    bigfixed_neg(result, result);
    TEST_APPROX("neg(-5.5) = 5.5", bigfixed_to_double(result), 5.5);
    
    BigFixed* b = bigfixed_from_double(-3.3, 64);
    bigfixed_abs(result, b);
    TEST_APPROX("abs(-3.3) = 3.3", bigfixed_to_double(result), 3.3);
    
    bigfixed_abs(result, a);
    TEST_APPROX("abs(5.5) = 5.5", bigfixed_to_double(result), 5.5);
    
    bigfixed_free(a);
    bigfixed_free(b);
    bigfixed_free(result);
}

int main(void) {
    printf("Crystalline Math Library - BigFixed Tests\n");
    printf("==========================================\n");
    
    test_creation_and_conversion();
    test_comparison();
    test_addition();
    test_subtraction();
    test_multiplication();
    test_division();
    test_transcendental();
    test_negation_and_abs();
    
    printf("\n==========================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);
    
    if (tests_failed > 0) {
        printf("FAILED: Some tests did not pass\n");
        return 1;
    }
    
    printf("SUCCESS: All tests passed!\n");
    return 0;
}