// clock_folding.c - Clock Folding Mode
// Shows how 2D clock folds to 3D sphere along sacred triangle

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_common.h"
#include "clock_abacus.h"
#include "../include/prime_math.h"

void draw_folding(SDL_Renderer* renderer, AppState* state) {
    // Mode 7: Clock Folding - Fold along sacred triangle
    int cx = RENDER_WIDTH / 2;
    int cy = WINDOW_HEIGHT / 2;
    double base_radius = 350.0 * state->zoom;
    double fold_amount = prime_sin(state->animation_phase * 0.05) * 0.5 + 0.5;
    
    // Draw sacred triangle
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 128);
    
    // Center to 12 o'clock
    double angle_12 = -PRIME_PI / 2.0;
    double radius_12 = get_ring_radius(0, base_radius);
    int x12 = cx + (int)(radius_12 * prime_cos(angle_12));
    int y12 = cy + (int)(radius_12 * prime_sin(angle_12));
    SDL_RenderDrawLine(renderer, cx, cy, x12, y12);
    
    // 12 o'clock to 3 o'clock
    double angle_3 = 0.0;
    double radius_3 = get_ring_radius(0, base_radius);
    int x3 = cx + (int)(radius_3 * prime_cos(angle_3));
    int y3 = cy + (int)(radius_3 * prime_sin(angle_3));
    SDL_RenderDrawLine(renderer, x12, y12, x3, y3);
    
    // 3 o'clock to center
    SDL_RenderDrawLine(renderer, x3, y3, cx, cy);
    
    // Draw rings with folding effect
    SDL_Color ring_colors[] = {
        {80, 80, 120, 255},
        {80, 120, 80, 255},
        {120, 80, 80, 255},
        {120, 120, 80, 255}
    };
    
    for (int ring = 0; ring < 4; ring++) {
        double radius = get_ring_radius(ring, base_radius);
        SDL_SetRenderDrawColor(renderer, ring_colors[ring].r, ring_colors[ring].g, ring_colors[ring].b, 255);
        
        for (int i = 0; i < 360; i++) {
            double angle = i * PRIME_PI / 180.0;
            
            // Apply folding transformation
            double x = radius * prime_cos(angle);
            double y = radius * prime_sin(angle);
            
            // Fold along triangle edge (y = -x + R)
            if (y > -x + radius_3) {
                // Above the fold line - lift into 3D
                double dist_from_fold = y + x - radius_3;
                double z = dist_from_fold * fold_amount;
                
                // Simple 3D projection
                double perspective = 1.0 / (1.0 + z / 300.0);
                x *= perspective;
                y *= perspective;
            }
            
            int px = cx + (int)x;
            int py = cy + (int)y;
            
            if (px >= 0 && px < RENDER_WIDTH && py >= 0 && py < WINDOW_HEIGHT) {
                SDL_RenderDrawPoint(renderer, px, py);
            }
        }
    }
    
    // Draw center
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int dy = -3; dy <= 3; dy++) {
        for (int dx = -3; dx <= 3; dx++) {
            if (dx*dx + dy*dy <= 9) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
    
    // Draw primes with folding
    int max_primes = 232;
    if (state->prime_count < max_primes) max_primes = state->prime_count;
    
    for (int i = 0; i < max_primes; i++) {
        int prime_index = i + 1;
        int ring = get_prime_ring(prime_index);
        int position = get_position_on_ring(prime_index, ring);
        double angle = get_clock_angle(position, ring);
        double radius = get_ring_radius(ring, base_radius);
        
        double x = radius * prime_cos(angle);
        double y = radius * prime_sin(angle);
        
        // Apply folding
        if (y > -x + radius_3) {
            double dist_from_fold = y + x - radius_3;
            double z = dist_from_fold * fold_amount;
            double perspective = 1.0 / (1.0 + z / 300.0);
            x *= perspective;
            y *= perspective;
        }
        
        int px = cx + (int)x + (int)state->offset_x;
        int py = cy + (int)y + (int)state->offset_y;
        
        if (px < -10 || px >= RENDER_WIDTH+10 || py < -10 || py >= WINDOW_HEIGHT+10) {
            continue;
        }
        
        SDL_Color color;
        if (ring == 0) color = (SDL_Color){200, 200, 255, 255};
        else if (ring == 1) color = (SDL_Color){200, 255, 200, 255};
        else if (ring == 2) color = (SDL_Color){255, 200, 200, 255};
        else color = (SDL_Color){255, 255, 200, 255};
        
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx*dx + dy*dy <= 4) {
                    int ppx = px + dx;
                    int ppy = py + dy;
                    if (ppx >= 0 && ppx < RENDER_WIDTH && ppy >= 0 && ppy < WINDOW_HEIGHT) {
                        SDL_RenderDrawPoint(renderer, ppx, ppy);
                    }
                }
            }
        }
    }
}