/**
 * Draw spheres in 3D mode with depth and perspective
 */
static void draw_spheres_3d(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds,
                           int center_x, int center_y, int max_batches) {
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color grid_color = {50, 50, 60, 255};
    
    // 3D parameters
    float rotation_angle = 0.0f; // Could be animated later
    int base_radius = (bounds.w < bounds.h ? bounds.w : bounds.h) / 4;
    int z_spacing = base_radius / 2;
    
    // Draw spheres in 3D arrangement (two rings of 6)
    for (int i = 0; i < 12; i++) {
        // Arrange in two rings: inner (0-5) and outer (6-11)
        int ring = i / 6;
        int pos_in_ring = i % 6;
        
        // Calculate 3D position
        float angle = (pos_in_ring * 2.0f * M_PI) / 6.0f + rotation_angle;
        float radius_3d = base_radius * (ring == 0 ? 0.7f : 1.0f);
        float z = (ring == 0 ? -z_spacing : z_spacing);
        
        // Apply perspective projection
        float perspective_scale = 1.0f / (1.0f + z / 500.0f);
        int sphere_x = center_x + (int)(radius_3d * prime_cosf(angle) * perspective_scale);
        int sphere_y = center_y + (int)(radius_3d * prime_sinf(angle) * perspective_scale);
        int sphere_radius = (int)(base_radius / 5 * perspective_scale);
        
        // Calculate activity level
        float activity = 0.0f;
        if (max_batches > 0) {
            activity = (float)state->sphere_stats.batches_processed[i] / (float)max_batches;
        }
        
        // Get color based on activity
        SDL_Color sphere_color = get_activity_color(activity);
        
        // Darken based on depth (further = darker)
        float depth_factor = (z + z_spacing) / (2.0f * z_spacing);
        sphere_color.r = (Uint8)(sphere_color.r * (0.5f + 0.5f * depth_factor));
        sphere_color.g = (Uint8)(sphere_color.g * (0.5f + 0.5f * depth_factor));
        sphere_color.b = (Uint8)(sphere_color.b * (0.5f + 0.5f * depth_factor));
        
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
        
        // Draw batch count
        if (state->sphere_stats.batches_processed[i] > 0) {
            char batch_text[16];
            snprintf(batch_text, sizeof(batch_text), "%d", state->sphere_stats.batches_processed[i]);
            int text_x = sphere_x + (int)((sphere_radius + 15) * prime_cosf(angle) * perspective_scale) - 10;
            int text_y = sphere_y + (int)((sphere_radius + 15) * prime_sinf(angle) * perspective_scale) - 6;
            draw_text(renderer, batch_text, text_x, text_y, (SDL_Color){180, 180, 180, 255});
        }
    }
}

/**
 * Draw spheres in 2D mode (flat circular arrangement)
 */
static void draw_spheres_2d(SDL_Renderer* renderer, AppState* state, SDL_Rect bounds,
                           int center_x, int center_y, int max_batches) {
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color grid_color = {50, 50, 60, 255};
    
    int arrangement_radius = (bounds.w < bounds.h ? bounds.w : bounds.h) / 3;
    int sphere_radius = arrangement_radius / 5;
    
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
}