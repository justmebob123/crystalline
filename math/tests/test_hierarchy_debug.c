#include "math/polytope_ntt.h"
#include "math/ntt.h"
#include <stdio.h>

int main(void) {
    printf("Testing hierarchy generation...\n");
    
    PlatonicSolid solid = {
        .dimension = 3,
        .num_vertices = 8
    };
    
    printf("Creating NTT context...\n");
    NTTContext* ctx = polytope_ntt_create_context(&solid);
    
    if (!ctx) {
        printf("Context creation failed!\n");
        return 1;
    }
    
    printf("Context created successfully.\n");
    
    printf("Generating hierarchy...\n");
    FaceHierarchy* hierarchy = NULL;
    MathError err = polytope_ntt_generate_hierarchy(&solid, ctx, &hierarchy);
    
    printf("Result: %d\n", err);
    
    if (err == MATH_SUCCESS && hierarchy) {
        printf("Hierarchy generated successfully!\n");
        printf("  - Polytope dimension: %u\n", hierarchy->polytope_dimension);
        printf("  - Num dimensions: %u\n", hierarchy->num_dimensions);
        
        for (uint32_t k = 0; k < hierarchy->num_dimensions; k++) {
            if (hierarchy->face_sets[k]) {
                printf("  - %u-faces: %u\n", k, hierarchy->face_sets[k]->count);
            }
        }
        
        /* Cleanup */
        for (uint32_t k = 0; k < hierarchy->num_dimensions; k++) {
            if (hierarchy->face_sets[k]) {
                for (uint32_t f = 0; f < hierarchy->face_sets[k]->count; f++) {
                    if (hierarchy->face_sets[k]->faces[f]) {
                        free(hierarchy->face_sets[k]->faces[f]->vertex_indices);
                        free(hierarchy->face_sets[k]->faces[f]);
                    }
                }
                free(hierarchy->face_sets[k]->faces);
                free(hierarchy->face_sets[k]);
            }
        }
        free(hierarchy->face_sets);
        free(hierarchy);
    } else {
        printf("Hierarchy generation failed! Error: %d\n", err);
    }
    
    ntt_free(ctx);
    
    return 0;
}