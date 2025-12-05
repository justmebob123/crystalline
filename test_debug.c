#include "include/ai/cllm_work_distribution.h"
#include "include/plimpton_322.h"
#include <stdio.h>

int main() {
    // Test with (3,2)
    printf("Testing (3,2):\n");
    
    PlimptonRatios ratios = calculate_plimpton_ratios(3, 2);
    printf("  b/d = %.6f\n", ratios.ratio_b_d);
    printf("  c/d = %.6f\n", ratios.ratio_c_d);
    printf("  sum = %.6f\n", ratios.ratio_b_d + ratios.ratio_c_d);
    
    WorkDistribution dist = calculate_work_distribution(3, 2);
    printf("  parent_keeps = %.6f\n", dist.parent_keeps);
    printf("  child_gets = %.6f\n", dist.child_gets);
    printf("  sum = %.6f\n", dist.parent_keeps + dist.child_gets);
    printf("  is_valid = %d\n", dist.is_valid);
    
    return 0;
}
