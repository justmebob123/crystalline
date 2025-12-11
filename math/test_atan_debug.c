#include "math.h"
#include <stdio.h>

int main() {
    double result = math_atan(1.0);
    double expected = MATH_PI / 4.0;
    
    printf("math_atan(1.0) = %.15f\n", result);
    printf("Expected (π/4) = %.15f\n", expected);
    printf("MATH_PI = %.15f\n", MATH_PI);
    printf("Difference = %.15e\n", result - expected);
    
    return 0;
}
