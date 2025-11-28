# Clock Lattice Implementation Plan

## Priority: CRITICAL
**The entire lattice structure must be rebuilt based on correct clock mathematics.**

---

## Phase 1: Core Clock Mapping Functions

### File: `src/geometry/clock_lattice.c` (NEW)

```c
#include "clock_lattice.h"
#include "prime_float_math.h"
#include <math.h>

// Clock position structure
typedef struct {
    int ring;           // 0=hours, 1=minutes, 2=seconds, 3=milliseconds
    int position;       // Position on that ring (1-based)
    double angle;       // Angle in radians (12 o'clock = -π/2, 3 o'clock = 0)
    double radius;      // Normalized radius (0.0 to 1.0)
} ClockPosition;

// Modular arithmetic relationships
typedef struct {
    uint64_t prime;
    int mod_12;         // prime % 12 (hours)
    int mod_60;         // prime % 60 (minutes/seconds)
    int mod_100;        // prime % 100 (milliseconds)
} PrimeModular;

// 3D sphere coordinates
typedef struct {
    float x, y, z;
} SphereCoord;

/**
 * Map prime index to clock position
 * 
 * Babylonian structure:
 * - Ring 0: Positions 1-12 (hours)
 * - Ring 1: Positions 1-60 (minutes)
 * - Ring 2: Positions 1-60 (seconds)
 * - Ring 3: Positions 1-100 (milliseconds)
 * 
 * @param prime_index 1-based prime index (1st prime, 2nd prime, etc.)
 * @return Clock position
 */
ClockPosition map_prime_index_to_clock(int prime_index) {
    ClockPosition pos;
    
    if (prime_index <= 12) {
        // Ring 0: Hours (12 positions)
        pos.ring = 0;
        pos.position = prime_index;
        // 12 o'clock = position 12, 3 o'clock = position 3
        // Angle: position 3 = 0°, position 12 = -90° (top)
        pos.angle = (pos.position - 3) * (2.0 * M_PI / 12.0);
        pos.radius = 0.25;  // Outer ring (25% from center)
        
    } else if (prime_index <= 72) {
        // Ring 1: Minutes (60 positions)
        pos.ring = 1;
        pos.position = prime_index - 12;
        // Position 15 = 3 o'clock (0°)
        pos.angle = (pos.position - 15) * (2.0 * M_PI / 60.0);
        pos.radius = 0.50;  // 50% from center
        
    } else if (prime_index <= 132) {
        // Ring 2: Seconds (60 positions)
        pos.ring = 2;
        pos.position = prime_index - 72;
        pos.angle = (pos.position - 15) * (2.0 * M_PI / 60.0);
        pos.radius = 0.75;  // 75% from center
        
    } else if (prime_index <= 232) {
        // Ring 3: Milliseconds (100 positions)
        pos.ring = 3;
        pos.position = prime_index - 132;
        // Position 25 = 3 o'clock (0°)
        pos.angle = (pos.position - 25) * (2.0 * M_PI / 100.0);
        pos.radius = 1.00;  // Inner ring (100% from center)
        
    } else {
        // Beyond 232: Continue pattern with larger rings
        pos.ring = 4;
        pos.position = prime_index - 232;
        pos.angle = (pos.position * 2.0 * M_PI) / 1000.0;
        pos.radius = 1.25;
    }
    
    return pos;
}

/**
 * Get modular arithmetic relationships for a prime
 */
PrimeModular get_prime_modular(uint64_t prime) {
    PrimeModular pm;
    pm.prime = prime;
    pm.mod_12 = prime % 12;
    pm.mod_60 = prime % 60;
    pm.mod_100 = prime % 100;
    return pm;
}

/**
 * Fold clock position into 3D sphere
 * 
 * Uses stereographic projection from first quadrant.
 * The clock folds along the sacred triangle (0→1→2→3→∞).
 * 
 * @param clock_pos Clock position
 * @return 3D sphere coordinates
 */
SphereCoord fold_clock_to_sphere(ClockPosition clock_pos) {
    SphereCoord sphere;
    
    // Normalize angle to [0, 2π]
    double theta = clock_pos.angle;
    while (theta < 0) theta += 2.0 * M_PI;
    while (theta >= 2.0 * M_PI) theta -= 2.0 * M_PI;
    
    // Fold into first quadrant (mirrored sudoku folding)
    // This maps all 4 quadrants into the sacred triangle
    double folded_theta = fmod(theta, M_PI / 2.0);
    
    // Radius determines latitude on sphere
    // Outer rings (small radius) → near north pole
    // Inner rings (large radius) → near equator
    double phi = clock_pos.radius * M_PI;  // 0 to π
    
    // Convert to 3D sphere coordinates (unit sphere)
    sphere.x = sin(phi) * cos(folded_theta);
    sphere.y = sin(phi) * sin(folded_theta);
    sphere.z = cos(phi);
    
    return sphere;
}

/**
 * Check if position is sacred (π, 12 o'clock, or related to 144,000)
 */
bool is_sacred_position(ClockPosition pos) {
    // 3 o'clock = π position
    if (pos.ring == 0 && pos.position == 3) return true;
    
    // 12 o'clock = top position
    if (pos.ring == 0 && pos.position == 12) return true;
    
    // Positions related to 144,000 = 12 × 12 × 1000
    // Any position divisible by 12 has special significance
    if (pos.position % 12 == 0) return true;
    
    return false;
}

/**
 * Map token to clock-based lattice coordinates
 * 
 * This is the CORRECT mapping that respects:
 * - Babylonian clock structure
 * - Modular arithmetic
 * - Sacred geometry
 * - Clock-to-sphere folding
 * 
 * @param token_id Token ID
 * @param prime Prime number for this token
 * @param prime_index 1-based index of this prime (1st, 2nd, 3rd, etc.)
 * @param coords Output: 3D coordinates [x, y, z]
 */
void map_token_to_clock_lattice(uint32_t token_id, uint64_t prime, 
                                int prime_index, float* coords) {
    // Get clock position
    ClockPosition clock_pos = map_prime_index_to_clock(prime_index);
    
    // Fold to 3D sphere
    SphereCoord sphere = fold_clock_to_sphere(clock_pos);
    
    // Store base 3D coordinates
    coords[0] = sphere.x;
    coords[1] = sphere.y;
    coords[2] = sphere.z;
    
    // Add modular arithmetic relationships as small perturbations
    // This encodes the prime's relationship to clock divisions
    PrimeModular pm = get_prime_modular(prime);
    
    // Small perturbation based on modular relationships
    // This creates subtle structure within each clock position
    coords[0] += 0.01f * (pm.mod_12 / 12.0f);
    coords[1] += 0.01f * (pm.mod_60 / 60.0f);
    coords[2] += 0.01f * (pm.mod_100 / 100.0f);
    
    // Normalize to unit sphere
    float norm = sqrt(coords[0]*coords[0] + coords[1]*coords[1] + coords[2]*coords[2]);
    if (norm > 1e-8f) {
        coords[0] /= norm;
        coords[1] /= norm;
        coords[2] /= norm;
    }
}
```

### File: `include/clock_lattice.h` (NEW)

```c
#ifndef CLOCK_LATTICE_H
#define CLOCK_LATTICE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Map token to clock-based lattice coordinates
 * 
 * @param token_id Token ID
 * @param prime Prime number for this token
 * @param prime_index 1-based index of this prime
 * @param coords Output: 3D coordinates [x, y, z]
 */
void map_token_to_clock_lattice(uint32_t token_id, uint64_t prime, 
                                int prime_index, float* coords);

#ifdef __cplusplus
}
#endif

#endif // CLOCK_LATTICE_H
```

---

## Phase 2: Update Lattice Embedding

### File: `src/ai/cllm_lattice_embed.c`

Replace `cllm_map_token_to_lattice()` function:

```c
#include "clock_lattice.h"  // Add this include

void cllm_map_token_to_lattice(uint32_t token_id, uint64_t prime, float* coords) {
    if (!coords) return;
    
    // Use clock-based mapping instead of spiral
    // prime_index = token_id + 1 (1-based indexing)
    map_token_to_clock_lattice(token_id, prime, token_id + 1, coords);
}
```

---

## Phase 3: Update Visualization

### File: `app/clock_crystalline.c`

Replace the visualization to show true clock structure:

```c
void draw_clock_sudoku_crystalline(SDL_Renderer* renderer, AppState* state) {
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    
    cx += (int)state->offset_x;
    cy += (int)state->offset_y;
    
    double base_radius = 350.0 * state->zoom;
    
    // Draw the four Babylonian rings
    SDL_Color ring_colors[] = {
        {80, 80, 120, 255},    // Ring 0: Hours (outer)
        {80, 120, 80, 255},    // Ring 1: Minutes
        {120, 80, 80, 255},    // Ring 2: Seconds
        {120, 120, 80, 255}    // Ring 3: Milliseconds (inner)
    };
    
    double ring_radii[] = {
        base_radius * 0.25,  // Hours (outer)
        base_radius * 0.50,  // Minutes
        base_radius * 0.75,  // Seconds
        base_radius * 1.00   // Milliseconds (inner)
    };
    
    // Draw rings
    for (int ring = 0; ring < 4; ring++) {
        SDL_SetRenderDrawColor(renderer, ring_colors[ring].r, 
                              ring_colors[ring].g, ring_colors[ring].b, 255);
        
        for (int i = 0; i < 360; i++) {
            double angle = i * M_PI / 180.0;
            int x = cx + (int)(ring_radii[ring] * cos(angle));
            int y = cy + (int)(ring_radii[ring] * sin(angle));
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    
    // Draw clock ticks (12, 3, 6, 9 o'clock)
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    for (int hour = 0; hour < 12; hour++) {
        double angle = (hour - 3) * (2.0 * M_PI / 12.0);  // 3 o'clock = 0°
        int x1 = cx + (int)(ring_radii[0] * 0.9 * cos(angle));
        int y1 = cy + (int)(ring_radii[0] * 0.9 * sin(angle));
        int x2 = cx + (int)(ring_radii[3] * 1.1 * cos(angle));
        int y2 = cy + (int)(ring_radii[3] * 1.1 * sin(angle));
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
    
    // Highlight sacred positions
    // 3 o'clock (π)
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);  // Gold
    double pi_angle = 0.0;  // 3 o'clock
    int pi_x = cx + (int)(ring_radii[0] * cos(pi_angle));
    int pi_y = cy + (int)(ring_radii[0] * sin(pi_angle));
    for (int dy = -5; dy <= 5; dy++) {
        for (int dx = -5; dx <= 5; dx++) {
            if (dx*dx + dy*dy <= 25) {
                SDL_RenderDrawPoint(renderer, pi_x + dx, pi_y + dy);
            }
        }
    }
    
    // 12 o'clock (top)
    double top_angle = -M_PI / 2.0;
    int top_x = cx + (int)(ring_radii[0] * cos(top_angle));
    int top_y = cy + (int)(ring_radii[0] * sin(top_angle));
    for (int dy = -5; dy <= 5; dy++) {
        for (int dx = -5; dx <= 5; dx++) {
            if (dx*dx + dy*dy <= 25) {
                SDL_RenderDrawPoint(renderer, top_x + dx, top_y + dy);
            }
        }
    }
    
    // Draw primes at their clock positions
    int max_primes = 232;  // First 232 primes (4 rings)
    if (state->prime_count < max_primes) max_primes = state->prime_count;
    
    for (int i = 0; i < max_primes; i++) {
        int prime_index = i + 1;  // 1-based
        
        // Get clock position
        ClockPosition pos = map_prime_index_to_clock(prime_index);
        
        // Calculate screen position
        int x = cx + (int)(ring_radii[pos.ring] * cos(pos.angle));
        int y = cy + (int)(ring_radii[pos.ring] * sin(pos.angle));
        
        // Color based on ring
        SDL_Color prime_color = ring_colors[pos.ring];
        prime_color.r += 100;
        prime_color.g += 100;
        prime_color.b += 100;
        
        // Highlight if sacred
        if (is_sacred_position(pos)) {
            prime_color = (SDL_Color){255, 255, 0, 255};  // Yellow
        }
        
        SDL_SetRenderDrawColor(renderer, prime_color.r, prime_color.g, 
                              prime_color.b, 255);
        
        // Draw prime dot
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx*dx + dy*dy <= 4) {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
    }
    
    // Draw center (unity)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            if (dx*dx + dy*dy <= 9) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}
```

---

## Phase 4: Testing

### Test 1: Verify Clock Positions

```c
// Test that primes map to correct clock positions
void test_clock_mapping() {
    // Prime #3 should be at 3 o'clock (π position)
    ClockPosition pos3 = map_prime_index_to_clock(3);
    assert(pos3.ring == 0);  // Hours ring
    assert(pos3.position == 3);  // 3 o'clock
    assert(fabs(pos3.angle - 0.0) < 0.01);  // 0 radians
    
    // Prime #12 should be at 12 o'clock (top)
    ClockPosition pos12 = map_prime_index_to_clock(12);
    assert(pos12.ring == 0);  // Hours ring
    assert(pos12.position == 12);  // 12 o'clock
    assert(fabs(pos12.angle - (-M_PI/2)) < 0.01);  // -π/2 radians
    
    // Prime #13 should be on minutes ring
    ClockPosition pos13 = map_prime_index_to_clock(13);
    assert(pos13.ring == 1);  // Minutes ring
    assert(pos13.position == 1);  // First position on minutes
    
    printf("✓ Clock mapping tests passed\n");
}
```

### Test 2: Verify Ring Structure

```c
void test_ring_structure() {
    // Verify ring boundaries
    assert(map_prime_index_to_clock(1).ring == 0);   // Hours
    assert(map_prime_index_to_clock(12).ring == 0);  // Hours
    assert(map_prime_index_to_clock(13).ring == 1);  // Minutes
    assert(map_prime_index_to_clock(72).ring == 1);  // Minutes
    assert(map_prime_index_to_clock(73).ring == 2);  // Seconds
    assert(map_prime_index_to_clock(132).ring == 2); // Seconds
    assert(map_prime_index_to_clock(133).ring == 3); // Milliseconds
    assert(map_prime_index_to_clock(232).ring == 3); // Milliseconds
    
    printf("✓ Ring structure tests passed\n");
}
```

### Test 3: Verify Modular Arithmetic

```c
void test_modular_arithmetic() {
    // Test prime modular relationships
    PrimeModular pm2 = get_prime_modular(2);
    assert(pm2.mod_12 == 2);
    assert(pm2.mod_60 == 2);
    assert(pm2.mod_100 == 2);
    
    PrimeModular pm13 = get_prime_modular(13);
    assert(pm13.mod_12 == 1);
    assert(pm13.mod_60 == 13);
    assert(pm13.mod_100 == 13);
    
    printf("✓ Modular arithmetic tests passed\n");
}
```

---

## Expected Results

After implementation:

1. ✅ Primes map to clock ticks (not spiral)
2. ✅ Smaller primes on outer rings (hours)
3. ✅ Larger primes on inner rings (milliseconds)
4. ✅ Babylonian structure visible (12, 60, 60, 100)
5. ✅ Sacred positions highlighted (π, 12 o'clock)
6. ✅ Modular arithmetic encoded in coordinates
7. ✅ Clock folds into sphere (3D coordinates)
8. ✅ 12-fold symmetry natural (from clock)

---

## Implementation Checklist

- [ ] Create `src/geometry/clock_lattice.c`
- [ ] Create `include/clock_lattice.h`
- [ ] Update `src/ai/cllm_lattice_embed.c`
- [ ] Update `app/clock_crystalline.c`
- [ ] Add to Makefile
- [ ] Write tests
- [ ] Run tests
- [ ] Verify visualization
- [ ] Update documentation

---

**This is the correct mathematical structure. All previous spiral-based mappings must be replaced.**