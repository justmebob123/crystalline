/**
 * Video Tab - Video Recording Controls
 * 
 * Features:
 * - Start/Stop recording
 * - Recording status display
 * - Frame counter
 * - File path display
 * - Recording settings
 */

#include "../../app_common.h"
#include "../../ui_layout.h"
#include <stdio.h>
#include <string.h>

// UI Button structure
typedef struct {
    SDL_Rect bounds;
    char label[64];
    bool enabled;
    bool visible;
} UIButton;

// Static UI elements
static UIButton btn_start_stop_recording;
static bool ui_initialized = false;

/**
 * Initialize video tab UI
 */
void init_video_tab(void) {
    ui_initialized = true;
}

/**
 * Draw video tab
 */
void draw_video_tab(SDL_Renderer* renderer, AppState* state) {
    if (!renderer || !state) return;
    
    // Colors
    SDL_Color bg_color = {20, 20, 30, 255};
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color panel_color = {30, 30, 40, 255};
    SDL_Color accent_color = {100, 150, 255, 255};
    SDL_Color success_color = {100, 255, 100, 255};
    SDL_Color error_color = {255, 100, 100, 255};
    SDL_Color recording_color = {255, 50, 50, 255};
    
    // Layout
    int x = 20;
    int y = 60;
    int w = RENDER_WIDTH - 40;
    int h = WINDOW_HEIGHT - 80;
    
    // Title
    draw_text(renderer, "VIDEO RECORDING", x, y, text_color);
    y += 40;
    
    // Recording status panel
    SDL_Rect status_panel = {x, y, w, 200};
    SDL_SetRenderDrawColor(renderer, panel_color.r, panel_color.g, panel_color.b, 255);
    SDL_RenderFillRect(renderer, &status_panel);
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderDrawRect(renderer, &status_panel);
    
    int panel_x = status_panel.x + 20;
    int panel_y = status_panel.y + 20;
    
    // Recording status
    if (state->is_recording) {
        draw_text(renderer, "STATUS: RECORDING", panel_x, panel_y, recording_color);
        panel_y += 25;
        
        // Blinking indicator
        if ((SDL_GetTicks() / 500) % 2 == 0) {
            SDL_Rect indicator = {panel_x, panel_y, 12, 12};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &indicator);
        }
        draw_text(renderer, "REC", panel_x + 20, panel_y, recording_color);
        panel_y += 30;
        
        // Frame counter
        char frame_info[128];
        snprintf(frame_info, sizeof(frame_info), "Frames Recorded: %d", state->recording_frames);
        draw_text(renderer, frame_info, panel_x, panel_y, text_color);
        panel_y += 20;
        
        // Duration
        int duration_seconds = state->recording_frames / 30;  // 30 FPS
        int minutes = duration_seconds / 60;
        int seconds = duration_seconds % 60;
        char duration_info[128];
        snprintf(duration_info, sizeof(duration_info), "Duration: %02d:%02d", minutes, seconds);
        draw_text(renderer, duration_info, panel_x, panel_y, text_color);
        panel_y += 20;
        
        // File path
        char path_info[512];
        snprintf(path_info, sizeof(path_info), "Output: %s", state->video_path);
        draw_text(renderer, path_info, panel_x, panel_y, accent_color);
        panel_y += 30;
        
        // Recording info
        draw_text(renderer, "Resolution: 1280x720 (720p HD)", panel_x, panel_y, text_color);
        panel_y += 18;
        draw_text(renderer, "Frame Rate: 30 FPS", panel_x, panel_y, text_color);
        panel_y += 18;
        draw_text(renderer, "Codec: H.264 (libx264)", panel_x, panel_y, text_color);
        
    } else {
        draw_text(renderer, "STATUS: READY", panel_x, panel_y, success_color);
        panel_y += 25;
        
        draw_text(renderer, "Press START RECORDING to begin", panel_x, panel_y, text_color);
        panel_y += 30;
        
        // Recording settings
        draw_text(renderer, "RECORDING SETTINGS:", panel_x, panel_y, accent_color);
        panel_y += 25;
        
        draw_text(renderer, "Resolution: 1280x720 (720p HD)", panel_x, panel_y, text_color);
        panel_y += 18;
        draw_text(renderer, "Frame Rate: 30 FPS", panel_x, panel_y, text_color);
        panel_y += 18;
        draw_text(renderer, "Codec: H.264 (libx264, CRF 18)", panel_x, panel_y, text_color);
        panel_y += 18;
        draw_text(renderer, "Format: MP4", panel_x, panel_y, text_color);
        panel_y += 30;
        
        // Instructions
        draw_text(renderer, "INSTRUCTIONS:", panel_x, panel_y, accent_color);
        panel_y += 25;
        draw_text(renderer, "1. Click START RECORDING button", panel_x, panel_y, text_color);
        panel_y += 18;
        draw_text(renderer, "2. Choose output file location", panel_x, panel_y, text_color);
        panel_y += 18;
        draw_text(renderer, "3. Recording captures all visualization tabs", panel_x, panel_y, text_color);
        panel_y += 18;
        draw_text(renderer, "4. Click STOP RECORDING when done", panel_x, panel_y, text_color);
        panel_y += 18;
        draw_text(renderer, "5. Video saved to chosen location", panel_x, panel_y, text_color);
    }
    
    y += 220;
    
    // Start/Stop Recording Button
    btn_start_stop_recording.bounds = (SDL_Rect){x + w/2 - 150, y, 300, 50};
    btn_start_stop_recording.enabled = true;
    btn_start_stop_recording.visible = true;
    
    SDL_Color button_color = state->is_recording ? error_color : success_color;
    SDL_SetRenderDrawColor(renderer, button_color.r, button_color.g, button_color.b, 255);
    SDL_RenderFillRect(renderer, &btn_start_stop_recording.bounds);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &btn_start_stop_recording.bounds);
    
    const char* button_text = state->is_recording ? "STOP RECORDING" : "START RECORDING";
    int text_w = strlen(button_text) * 10;
    draw_text(renderer, button_text, 
             btn_start_stop_recording.bounds.x + btn_start_stop_recording.bounds.w/2 - text_w/2,
             btn_start_stop_recording.bounds.y + 18,
             (SDL_Color){255, 255, 255, 255});
    
    y += 70;
    
    // Requirements panel
    SDL_Rect req_panel = {x, y, w, 150};
    SDL_SetRenderDrawColor(renderer, panel_color.r, panel_color.g, panel_color.b, 255);
    SDL_RenderFillRect(renderer, &req_panel);
    SDL_SetRenderDrawColor(renderer, 60, 60, 70, 255);
    SDL_RenderDrawRect(renderer, &req_panel);
    
    int req_x = req_panel.x + 20;
    int req_y = req_panel.y + 20;
    
    draw_text(renderer, "REQUIREMENTS:", req_x, req_y, accent_color);
    req_y += 25;
    
    // Check if ffmpeg is available
    static int ffmpeg_checked = 0;
    static bool ffmpeg_available = false;
    
    if (ffmpeg_checked == 0) {
        ffmpeg_available = (system("which ffmpeg > /dev/null 2>&1") == 0);
        ffmpeg_checked = 1;
    }
    
    if (ffmpeg_available) {
        draw_text(renderer, "✓ FFmpeg: Installed", req_x, req_y, success_color);
    } else {
        draw_text(renderer, "✗ FFmpeg: Not Found", req_x, req_y, error_color);
        req_y += 18;
        draw_text(renderer, "  Install: sudo apt-get install ffmpeg", req_x, req_y, text_color);
    }
    req_y += 25;
    
    draw_text(renderer, "NOTES:", req_x, req_y, accent_color);
    req_y += 25;
    draw_text(renderer, "• Recording captures the main visualization area", req_x, req_y, text_color);
    req_y += 18;
    draw_text(renderer, "• Switch between tabs while recording to capture different views", req_x, req_y, text_color);
    req_y += 18;
    draw_text(renderer, "• Recording may impact performance slightly", req_x, req_y, text_color);
}

/**
 * Handle video tab mouse click
 */
void handle_video_tab_click(AppState* state, int x, int y) {
    if (!state) return;
    
    // Check if Start/Stop Recording button clicked
    if (x >= btn_start_stop_recording.bounds.x && 
        x <= btn_start_stop_recording.bounds.x + btn_start_stop_recording.bounds.w &&
        y >= btn_start_stop_recording.bounds.y && 
        y <= btn_start_stop_recording.bounds.y + btn_start_stop_recording.bounds.h) {
        
        if (state->is_recording) {
            printf("Stopping recording...\n");
            extern void stop_recording(AppState* state);
            stop_recording(state);
        } else {
            printf("Starting recording...\n");
            extern void start_recording(AppState* state);
            start_recording(state);
        }
    }
}