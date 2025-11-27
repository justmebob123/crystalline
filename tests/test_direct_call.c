#include <stdio.h>

// Declare the function directly
typedef struct OptimizerState OptimizerState;
extern int optimizer_step(OptimizerState* state, const float* gradients);

int main() {
    printf("Calling optimizer_step with NULL pointers\n");
    int result = optimizer_step(NULL, NULL);
    printf("Result: %d (expected -1)\n", result);
    return 0;
}