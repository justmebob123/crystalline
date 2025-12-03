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
    
    // Calculate center
    int center_x = bounds.x + bounds.w / 2;
    int center_y = bounds.y + bounds.h / 2;
    int arrangement_radius = (bounds.w < bounds.h ? bounds.w : bounds.h) / 3;
    int sphere_radius = arrangement_radius / 5;
    
    // Draw title with mode indicator
    char title[128];
    snprintf(title, sizeof(title), "KISSING SPHERES ARCHITECTURE [%s Mode]",
             state->sphere_viz_mode == SPHERE_VIZ_3D ? "3D" : "2D");
    draw_text(renderer, title, bounds.x + 10, bounds.y + 10, text_color);
    
    // Lock sphere_stats for reading
    pthread_mutex_lock(&state->sphere_stats_mutex);
    
    // UI Integration: Draw subtitle with real-time status
    if (state->training_metrics) {
        char status_text[128];
        snprintf(status_text, sizeof(status_text), "Real-time Metrics | Epoch %d | Loss: %.4f",
                state->training_current_epoch, state->training_loss);
        draw_text(renderer, status_text, bounds.x + 10, bounds.y + 30, 
                 (SDL_Color){150, 150, 150, 255});
    }
    
    // Draw center sphere (Node Zero - Control Thread)
    draw_filled_circle(renderer, center_x, center_y, sphere_radius / 2, center_color);
    draw_circle_outline(renderer, center_x, center_y, sphere_radius / 2, text_color);
    draw_text(renderer, "0", center_x - 4, center_y - 6, text_color);
    
    // Find max batch count for normalization
    int max_batches = 1;
    for (int i = 0; i < 12; i++) {
        if (state->sphere_stats.batches_processed[i] > max_batches) {
            max_batches = state->sphere_stats.batches_processed[i];
        }
    }
    
    // Draw spheres based on current mode
    if (state->sphere_viz_mode == SPHERE_VIZ_3D) {
        draw_spheres_3d(renderer, state, bounds, center_x, center_y, max_batches);
    } else {
        draw_spheres_2d(renderer, state, bounds, center_x, center_y, max_batches);
    }
    
    // Draw statistics panel at bottom
    int stats_y = bounds.y + bounds.h - 110;
    SDL_Rect stats_panel = {bounds.x + 10, stats_y, bounds.w - 20, 100};
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
    
    // Unlock sphere_stats after reading all data
    pthread_mutex_unlock(&state->sphere_stats_mutex);
    
    // Draw legend
    int legend_x = stats_panel.x + stats_panel.w - 200;
    int legend_y = stats_panel.y + 8;
    
    draw_text(renderer, "Activity Level:", legend_x, legend_y, text_color);
    legend_y += 18;
    
    // Color legend with labels
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
        // Draw color box
        SDL_Rect color_box = {legend_x, legend_y, 12, 12};
        SDL_SetRenderDrawColor(renderer, legend_items[i].color.r, 
                              legend_items[i].color.g, 
                              legend_items[i].color.b, 255);
        SDL_RenderFillRect(renderer, &color_box);
        SDL_SetRenderDrawColor(renderer, text_color.r, text_color.g, text_color.b, 255);
        SDL_RenderDrawRect(renderer, &color_box);
        
        // Draw label
        draw_text(renderer, legend_items[i].label, legend_x + 18, legend_y, 
                 (SDL_Color){180, 180, 180, 255});
        legend_y += 16;
    }
}