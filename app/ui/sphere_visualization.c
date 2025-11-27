/**
 * Sphere Visualization Module
 * 
 * Displays the 12 kissing spheres in a circular arrangement
 * with real-time activity indicators, batch counts, and loss values.
 */

#include "../app_common.h"
#include "../../include/prime_float_math.h"
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Draw a filled circle (sphere representation)
 */
static void draw_filled_circle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}

/**
 * Draw a circle outline
 */
static void draw_circle_outline(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    int x = radius;
    int y = 0;
    int err = 0;
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    while (x >= y) {
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

/**
 * Get color based on activity level (0.0 to 1.0)
 */
static SDL_Color get_activity_color(float activity) {
    SDL_Color color;
    
    if (activity < 0.01f) {
        // Inactive - dark gray
        color = (SDL_Color){40, 40, 50, 255};
    } else if (activity < 0.3f) {
        // Low activity - blue
        color = (SDL_Color){60, 100, 180, 255};
    } else if (activity < 0.6f) {
        // Medium activity - cyan
        color = (SDL_Color){80, 180, 200, 255};
    } else if (activity < 0.9f) {
        // High activity - yellow
        color = (SDL_Color){220, 200, 80, 255};
    } else {
        // Very high activity - orange/red
        color = (SDL_Color){255, 140, 60, 255};
    }
    
    return color;
}

/**
 * Draw the 12 kissing spheres in a circular arrangement
 */
void draw_sphere_visualization(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds) {
    if (!state) return;
    
    SDL_Color bg_color = {25, 25, 30, 255};
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color grid_color = {50, 50, 60, 255};
    SDL_Color center_color = {100, 100, 120, 255};
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &bounds);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
    SDL_RenderDrawRect(renderer, &bounds);
    
    // Calculate center and radius for sphere arrangement
    int center_x = bounds.x + bounds.w / 2;
    int center_y = bounds.y + bounds.h / 2;
    int arrangement_radius = (bounds.w < bounds.h ? bounds.w : bounds.h) / 3;
    int sphere_radius = arrangement_radius / 5;
    
    // Draw title
    draw_text(renderer, "KISSING SPHERES ARCHITECTURE", bounds.x + 10, bounds.y + 10, text_color);
    
    // Draw center sphere (coordinator)
    draw_filled_circle(renderer, center_x, center_y, sphere_radius / 2, center_color);
    draw_circle_outline(renderer, center_x, center_y, sphere_radius / 2, text_color);
    
    // Find max batch count for normalization
    int max_batches = 1;
    for (int i = 0; i < 12; i++) {
        if (state->sphere_stats.batches_processed[i] > max_batches) {
            max_batches = state->sphere_stats.batches_processed[i];
        }
    }
    
    // Draw 12 worker spheres in a circle
    for (int i = 0; i < 12; i++) {
        // Calculate position
        float angle = (i * 2.0f * M_PI) / 12.0f - M_PI / 2.0f; // Start at top
        int sphere_x = center_x + (int)(arrangement_radius * prime_cosf(angle));
        int sphere_y = center_y + (int)(arrangement_radius * prime_sinf(angle));
        
        // Calculate activity level (0.0 to 1.0)
        float activity = 0.0f;
        if (max_batches > 0) {
            activity = (float)state->sphere_stats.batches_processed[i] / (float)max_batches;
        }
        
        // Get color based on activity
        SDL_Color sphere_color = get_activity_color(activity);
        
        // Draw sphere
        draw_filled_circle(renderer, sphere_x, sphere_y, sphere_radius, sphere_color);
        draw_circle_outline(renderer, sphere_x, sphere_y, sphere_radius, text_color);
        
        // Draw connection line to center
        SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 128);
        SDL_RenderDrawLine(renderer, center_x, center_y, sphere_x, sphere_y);
        
        // Draw sphere number
        char sphere_label[8];
        snprintf(sphere_label, sizeof(sphere_label), "%d", i);
        draw_text(renderer, sphere_label, sphere_x - 4, sphere_y - 6, text_color);
        
        // Draw batch count below sphere
        if (state->sphere_stats.batches_processed[i] > 0) {
            char batch_text[16];
            snprintf(batch_text, sizeof(batch_text), "%d", state->sphere_stats.batches_processed[i]);
            
            // Position text outside the circle
            int text_x = sphere_x + (int)((sphere_radius + 15) * prime_cosf(angle)) - 10;
            int text_y = sphere_y + (int)((sphere_radius + 15) * prime_sinf(angle)) - 6;
            
            draw_text(renderer, batch_text, text_x, text_y, 
                     (SDL_Color){180, 180, 180, 255});
        }
    }
    
    // Draw statistics panel at bottom
    int stats_y = bounds.y + bounds.h - 80;
    SDL_Rect stats_panel = {bounds.x + 10, stats_y, bounds.w - 20, 70};
    SDL_SetRenderDrawColor(renderer, 35, 35, 40, 255);
    SDL_RenderFillRect(renderer, &stats_panel);
    SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
    SDL_RenderDrawRect(renderer, &stats_panel);
    
    // Display statistics
    char stats_text[256];
    int text_y = stats_panel.y + 8;
    
    snprintf(stats_text, sizeof(stats_text), "Active Spheres: %d / 12", 
             state->sphere_stats.active_spheres);
    draw_text(renderer, stats_text, stats_panel.x + 10, text_y, text_color);
    text_y += 18;
    
    snprintf(stats_text, sizeof(stats_text), "Total Batches: %d", 
             state->sphere_stats.total_batches);
    draw_text(renderer, stats_text, stats_panel.x + 10, text_y, text_color);
    text_y += 18;
    
    snprintf(stats_text, sizeof(stats_text), "Gradient Norm: %.4f", 
             state->sphere_stats.total_gradient_norm);
    draw_text(renderer, stats_text, stats_panel.x + 10, text_y, text_color);
    
    // Draw legend
    int legend_x = stats_panel.x + stats_panel.w - 200;
    int legend_y = stats_panel.y + 8;
    
    draw_text(renderer, "Activity:", legend_x, legend_y, text_color);
    legend_y += 18;
    
    // Color legend
    struct {
        const char* label;
        SDL_Color color;
    } legend_items[] = {
        {"Inactive", {40, 40, 50, 255}},
        {"Low", {60, 100, 180, 255}},
        {"Medium", {80, 180, 200, 255}},
        {"High", {220, 200, 80, 255}},
        {"Very High", {255, 140, 60, 255}}
    };
    
    for (int i = 0; i < 5; i++) {
        int item_x = legend_x + (i * 35);
        draw_filled_circle(renderer, item_x + 5, legend_y + 5, 4, legend_items[i].color);
    }
}

/**
 * Draw detailed per-sphere statistics table
 */
void draw_sphere_stats_table(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds) {
    if (!state) return;
    
    SDL_Color bg_color = {25, 25, 30, 255};
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color header_color = {100, 150, 200, 255};
    SDL_Color grid_color = {50, 50, 60, 255};
    
    // Draw background
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, 255);
    SDL_RenderFillRect(renderer, &bounds);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
    SDL_RenderDrawRect(renderer, &bounds);
    
    int x = bounds.x + 10;
    int y = bounds.y + 10;
    
    // Title
    draw_text(renderer, "PER-SPHERE STATISTICS", x, y, header_color);
    y += 25;
    
    // Table header
    draw_text(renderer, "Sphere", x, y, header_color);
    draw_text(renderer, "Batches", x + 80, y, header_color);
    draw_text(renderer, "Avg Loss", x + 160, y, header_color);
    draw_text(renderer, "Status", x + 250, y, header_color);
    y += 20;
    
    // Draw horizontal line
    SDL_SetRenderDrawColor(renderer, grid_color.r, grid_color.g, grid_color.b, 255);
    SDL_RenderDrawLine(renderer, x, y, x + bounds.w - 20, y);
    y += 5;
    
    // Draw data for each sphere
    for (int i = 0; i < 12; i++) {
        char sphere_num[8];
        snprintf(sphere_num, sizeof(sphere_num), "#%d", i);
        draw_text(renderer, sphere_num, x, y, text_color);
        
        char batches[16];
        snprintf(batches, sizeof(batches), "%d", state->sphere_stats.batches_processed[i]);
        draw_text(renderer, batches, x + 80, y, text_color);
        
        char loss[16];
        if (state->sphere_stats.batches_processed[i] > 0) {
            snprintf(loss, sizeof(loss), "%.4f", state->sphere_stats.avg_loss[i]);
        } else {
            snprintf(loss, sizeof(loss), "-");
        }
        draw_text(renderer, loss, x + 160, y, text_color);
        
        const char* status = state->sphere_stats.batches_processed[i] > 0 ? "Active" : "Idle";
        SDL_Color status_color = state->sphere_stats.batches_processed[i] > 0 ? 
            (SDL_Color){100, 255, 100, 255} : (SDL_Color){100, 100, 100, 255};
        draw_text(renderer, status, x + 250, y, status_color);
        
        y += 16;
        
        // Stop if we run out of space
        if (y > bounds.y + bounds.h - 20) break;
    }
}