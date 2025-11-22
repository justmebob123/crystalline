// clock_crystalline.c - Crystalline Lattice Clock Transformation
// ULTRA-FAST VERSION - No expensive library calls, pure geometric transformation

#include "app_common.h"

// Simple, fast crystalline transformation without expensive library calls
// This uses a simplified version that's fast enough for real-time rendering
void get_crystalline_position_fast(int prime, uint64_t prime_index, double *angle, double *radius, AppState* state) {
    // Simple transformation based on prime index and value
    // This is MUCH faster than calling theta_n() and r_n()
    
    // Base angle from prime index (distributes around clock)
    double base_angle = ((double)prime_index * 2.0 * PRIME_PI / 12.0);
    
    // Add offset based on prime value (creates spiral)
    double prime_offset = ((double)(prime % 360)) * (PRIME_PI / 180.0);
    
    // Combine for final angle
    *angle = base_angle + prime_offset * 0.1;
    
    // Normalize to 0-2π
    double two_pi = 2.0 * PRIME_PI;
    while (*angle < 0) *angle += two_pi;
    while (*angle >= two_pi) *angle -= two_pi;
    
    // Simple radius based on prime magnitude
    double log_prime = prime_log((double)prime) / prime_log(10.0);
    *radius = (log_prime * 100.0) * state->zoom;
}

// Draw clock sudoku with crystalline lattice transformation
// PURE PRIME VISUALIZATION - No manual lines or rings!
void draw_clock_sudoku_crystalline(SDL_Renderer* renderer, AppState* state) {
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    
    // Apply offset
    cx += (int)state->offset_x;
    cy += (int)state->offset_y;
    
    // Draw ONLY the primes - let them form the pattern naturally!
    // NO manual lines, NO manual rings - pure prime distribution
    
    for (int i = 0; i < state->visible_primes && i < MAX_PRIMES; i++) {
        int prime = state->primes[i];
        uint64_t prime_index = (uint64_t)(i + 1);
        
        double angle, radius;
        get_crystalline_position_fast(prime, prime_index, &angle, &radius, state);
        
        // Calculate screen position
        int x = cx + (int)(radius * prime_cos(angle));
        int y = cy + (int)(radius * prime_sin(angle));
        
        // Skip if off-screen
        if (x < -20 || x >= RENDER_WIDTH + 20 || y < -20 || y >= WINDOW_HEIGHT + 20) {
            continue;
        }
        
        // Color based on magnitude
        SDL_Color prime_color;
        if (prime < 100) {
            prime_color = (SDL_Color){150, 220, 255, 255};  // Blue
        } else if (prime < 1000) {
            prime_color = (SDL_Color){150, 255, 200, 255};  // Green
        } else {
            prime_color = (SDL_Color){255, 220, 150, 255};  // Orange
        }
        
        // Highlight analyzed number
        if (state->number_analysis.number == prime) {
            prime_color = (SDL_Color){255, 255, 0, 255};  // Bright yellow
            // Draw larger for visibility
            SDL_SetRenderDrawColor(renderer, prime_color.r, prime_color.g, prime_color.b, prime_color.a);
            for (int dy = -5; dy <= 5; dy++) {
                for (int dx = -5; dx <= 5; dx++) {
                    if (dx*dx + dy*dy <= 25) {
                        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                    }
                }
            }
            continue;
        }
        
        SDL_SetRenderDrawColor(renderer, prime_color.r, prime_color.g, prime_color.b, prime_color.a);
        
        // Draw prime as filled circle
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx*dx + dy*dy <= 4) {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }
    }
    
    // Draw center point
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
        }
    }
    
    // Draw analyzed number separately (even if not in primes array)
    if (state->number_analysis.number > 0) {
        int analyzed_num = state->number_analysis.number;
        
        // Calculate position using EXACT same crystalline transformation as primes
        uint64_t prime_index = 0;
        // Find approximate index (for positioning)
        for (int i = 0; i < state->prime_count; i++) {
            if (state->primes[i] >= analyzed_num) {
                prime_index = (uint64_t)i;
                break;
            }
        }
        if (prime_index == 0) prime_index = (uint64_t)state->prime_count;
        
        // Use EXACT same function as regular primes
        double angle, radius;
        get_crystalline_position_fast(analyzed_num, prime_index, &angle, &radius, state);
        
        int x = cx + (int)(radius * prime_cos(angle));
        int y = cy + (int)(radius * prime_sin(angle));
        
        // Draw with bright yellow/gold highlighting (extra large)
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        
        // Draw large highlighted point (12 pixel radius)
        for (int dy = -12; dy <= 12; dy++) {
            for (int dx = -12; dx <= 12; dx++) {
                if (dx*dx + dy*dy <= 144) {
                    int px = x + dx;
                    int py = y + dy;
                    if (px >= 0 && px < RENDER_WIDTH && py >= 0 && py < WINDOW_HEIGHT) {
                        SDL_RenderDrawPoint(renderer, px, py);
                    }
                }
            }
        }
        
        // Draw pulsing ring around it
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 200);
        for (int r = 14; r <= 18; r++) {
            for (int a = 0; a < 360; a += 5) {
                int px = x + (int)(r * prime_cos(a * PRIME_PI / 180.0));
                int py = y + (int)(r * prime_sin(a * PRIME_PI / 180.0));
                if (px >= 0 && px < RENDER_WIDTH && py >= 0 && py < WINDOW_HEIGHT) {
                    SDL_RenderDrawPoint(renderer, px, py);
                }
            }
        }
    }
    
    // Draw label
    SDL_Color label_color = {100, 255, 100, 255};
    draw_text(renderer, "CRYSTALLINE LATTICE (Pure Prime Distribution)", 50, WINDOW_HEIGHT - 30, label_color);
}