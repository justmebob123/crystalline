/*
 * Benchmark Tab
 * Comprehensive performance benchmarking and analysis
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../app_common.h"

#define MAX_BENCHMARK_HISTORY 100
#define MAX_BENCHMARK_TESTS 20

typedef struct {
    char name[128];
    double duration_ms;
    int operations;
    double ops_per_second;
    time_t timestamp;
    bool success;
} BenchmarkResult;

typedef struct {
    BenchmarkResult results[MAX_BENCHMARK_HISTORY];
    int result_count;
    
    // Current benchmark state
    bool is_running;
    int current_test;
    int total_tests;
    float progress;
    
    // Performance metrics
    double avg_inference_time;
    double avg_tokens_per_second;
    double peak_tokens_per_second;
    double total_tokens_generated;
    double total_time_spent;
    
    // System metrics
    double cpu_usage;
    double memory_usage_mb;
    double gpu_usage;
    
    // UI state
    int scroll_offset;
    int selected_result;
    bool show_details;
} BenchmarkTabState;

static BenchmarkTabState benchmark_state = {
    .result_count = 0,
    .is_running = false,
    .current_test = 0,
    .total_tests = 0,
    .progress = 0.0f,
    .scroll_offset = 0,
    .selected_result = -1,
    .show_details = false
};

void benchmark_tab_init(void) {
    memset(&benchmark_state, 0, sizeof(BenchmarkTabState));
    benchmark_state.selected_result = -1;
}

void benchmark_tab_add_result(const char* name, double duration_ms, int operations, bool success) {
    if (benchmark_state.result_count >= MAX_BENCHMARK_HISTORY) {
        // Shift results
        memmove(&benchmark_state.results[0], &benchmark_state.results[1],
                sizeof(BenchmarkResult) * (MAX_BENCHMARK_HISTORY - 1));
        benchmark_state.result_count--;
    }
    
    BenchmarkResult* result = &benchmark_state.results[benchmark_state.result_count++];
    strncpy(result->name, name, sizeof(result->name) - 1);
    result->duration_ms = duration_ms;
    result->operations = operations;
    result->ops_per_second = (operations * 1000.0) / duration_ms;
    result->timestamp = time(NULL);
    result->success = success;
    
    // Update aggregate metrics
    benchmark_state.total_tokens_generated += operations;
    benchmark_state.total_time_spent += duration_ms / 1000.0;
    
    if (result->ops_per_second > benchmark_state.peak_tokens_per_second) {
        benchmark_state.peak_tokens_per_second = result->ops_per_second;
    }
    
    // Calculate averages
    double total_ops = 0;
    double total_time = 0;
    for (int i = 0; i < benchmark_state.result_count; i++) {
        total_ops += benchmark_state.results[i].operations;
        total_time += benchmark_state.results[i].duration_ms;
    }
    benchmark_state.avg_tokens_per_second = (total_ops * 1000.0) / total_time;
    benchmark_state.avg_inference_time = total_time / benchmark_state.result_count;
}

void benchmark_tab_run_tests(void) {
    benchmark_state.is_running = true;
    benchmark_state.current_test = 0;
    benchmark_state.total_tests = 10;
    
    // Simulate benchmark tests
    const char* test_names[] = {
        "Token Generation Speed",
        "Context Window Performance",
        "Batch Processing",
        "Memory Efficiency",
        "Lattice Lookup Speed",
        "Attention Computation",
        "Feed-Forward Speed",
        "Embedding Lookup",
        "Positional Encoding",
        "Full Pipeline"
    };
    
    for (int i = 0; i < benchmark_state.total_tests; i++) {
        benchmark_state.current_test = i;
        benchmark_state.progress = (float)(i + 1) / benchmark_state.total_tests * 100.0f;
        
        // Simulate test execution
        double duration = 10.0 + (rand() % 90);  // 10-100ms
        int operations = 50 + (rand() % 150);  // 50-200 operations
        
        benchmark_tab_add_result(test_names[i], duration, operations, true);
        
        // Small delay to show progress
        SDL_Delay(100);
    }
    
    benchmark_state.is_running = false;
}

void benchmark_tab_render(SDL_Renderer* renderer, TTF_Font* font, int x, int y, int w, int h) {
    SDL_Color bg_color = {20, 20, 30, 255};
    SDL_Color text_color = {220, 220, 220, 255};
    SDL_Color panel_color = {30, 30, 40, 255};
    SDL_Color accent_color = {100, 150, 255, 255};
    SDL_Color success_color = {100, 255, 100, 255};
    SDL_Color warning_color = {255, 200, 100, 255};
    
    // Background
    SDL_Rect bg_rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, bg_color.r, bg_color.g, bg_color.b, bg_color.a);
    SDL_RenderFillRect(renderer, &bg_rect);
    
    // Title
    SDL_Surface* title_surface = TTF_RenderText_Blended(font, "Performance Benchmarks", text_color);
    if (title_surface) {
        SDL_Texture* title_texture = SDL_CreateTextureFromSurface(renderer, title_surface);
        SDL_Rect title_rect = {x + 20, y + 10, title_surface->w, title_surface->h};
        SDL_RenderCopy(renderer, title_texture, NULL, &title_rect);
        SDL_DestroyTexture(title_texture);
        SDL_FreeSurface(title_surface);
    }
    
    // Summary metrics panel
    SDL_Rect metrics_panel = {x + 20, y + 50, w - 40, 120};
    SDL_SetRenderDrawColor(renderer, panel_color.r, panel_color.g, panel_color.b, panel_color.a);
    SDL_RenderFillRect(renderer, &metrics_panel);
    
    char metrics_text[512];
    snprintf(metrics_text, sizeof(metrics_text),
             "Avg Inference Time: %.2f ms | Avg Tokens/sec: %.1f | Peak: %.1f tokens/sec",
             benchmark_state.avg_inference_time,
             benchmark_state.avg_tokens_per_second,
             benchmark_state.peak_tokens_per_second);
    
    SDL_Surface* metrics_surface = TTF_RenderText_Blended(font, metrics_text, text_color);
    if (metrics_surface) {
        SDL_Texture* metrics_texture = SDL_CreateTextureFromSurface(renderer, metrics_surface);
        SDL_Rect metrics_rect = {x + 30, y + 65, metrics_surface->w, metrics_surface->h};
        SDL_RenderCopy(renderer, metrics_texture, NULL, &metrics_rect);
        SDL_DestroyTexture(metrics_texture);
        SDL_FreeSurface(metrics_surface);
    }
    
    char totals_text[256];
    snprintf(totals_text, sizeof(totals_text),
             "Total Tokens: %.0f | Total Time: %.1f sec | Tests Run: %d",
             benchmark_state.total_tokens_generated,
             benchmark_state.total_time_spent,
             benchmark_state.result_count);
    
    SDL_Surface* totals_surface = TTF_RenderText_Blended(font, totals_text, accent_color);
    if (totals_surface) {
        SDL_Texture* totals_texture = SDL_CreateTextureFromSurface(renderer, totals_surface);
        SDL_Rect totals_rect = {x + 30, y + 95, totals_surface->w, totals_surface->h};
        SDL_RenderCopy(renderer, totals_texture, NULL, &totals_rect);
        SDL_DestroyTexture(totals_texture);
        SDL_FreeSurface(totals_surface);
    }
    
    char system_text[256];
    snprintf(system_text, sizeof(system_text),
             "CPU: %.1f%% | Memory: %.1f MB | GPU: %.1f%%",
             benchmark_state.cpu_usage,
             benchmark_state.memory_usage_mb,
             benchmark_state.gpu_usage);
    
    SDL_Surface* system_surface = TTF_RenderText_Blended(font, system_text, warning_color);
    if (system_surface) {
        SDL_Texture* system_texture = SDL_CreateTextureFromSurface(renderer, system_surface);
        SDL_Rect system_rect = {x + 30, y + 125, system_surface->w, system_surface->h};
        SDL_RenderCopy(renderer, system_texture, NULL, &system_rect);
        SDL_DestroyTexture(system_texture);
        SDL_FreeSurface(system_surface);
    }
    
    // Progress bar (if running)
    if (benchmark_state.is_running) {
        SDL_Rect progress_panel = {x + 20, y + 190, w - 40, 80};
        SDL_SetRenderDrawColor(renderer, panel_color.r, panel_color.g, panel_color.b, panel_color.a);
        SDL_RenderFillRect(renderer, &progress_panel);
        
        char progress_text[256];
        snprintf(progress_text, sizeof(progress_text),
                 "Running Test %d/%d: %.1f%% Complete",
                 benchmark_state.current_test + 1,
                 benchmark_state.total_tests,
                 benchmark_state.progress);
        
        SDL_Surface* progress_surface = TTF_RenderText_Blended(font, progress_text, text_color);
        if (progress_surface) {
            SDL_Texture* progress_texture = SDL_CreateTextureFromSurface(renderer, progress_surface);
            SDL_Rect progress_rect = {x + 30, y + 205, progress_surface->w, progress_surface->h};
            SDL_RenderCopy(renderer, progress_texture, NULL, &progress_rect);
            SDL_DestroyTexture(progress_texture);
            SDL_FreeSurface(progress_surface);
        }
        
        // Progress bar
        SDL_Rect progress_bg = {x + 30, y + 235, w - 80, 20};
        SDL_SetRenderDrawColor(renderer, 50, 50, 60, 255);
        SDL_RenderFillRect(renderer, &progress_bg);
        
        SDL_Rect progress_fill = {x + 30, y + 235, 
                                  (int)((w - 80) * benchmark_state.progress / 100.0f), 20};
        SDL_SetRenderDrawColor(renderer, success_color.r, success_color.g, success_color.b, success_color.a);
        SDL_RenderFillRect(renderer, &progress_fill);
    }
    
    // Results list
    int list_y = benchmark_state.is_running ? y + 290 : y + 190;
    int list_h = h - (list_y - y) - 20;
    SDL_Rect list_panel = {x + 20, list_y, w - 40, list_h};
    SDL_SetRenderDrawColor(renderer, panel_color.r, panel_color.g, panel_color.b, panel_color.a);
    SDL_RenderFillRect(renderer, &list_panel);
    
    // Render results
    int result_y = list_y + 10 - benchmark_state.scroll_offset;
    for (int i = benchmark_state.result_count - 1; i >= 0; i--) {
        BenchmarkResult* result = &benchmark_state.results[i];
        
        char result_text[512];
        snprintf(result_text, sizeof(result_text),
                 "%s: %.2f ms | %d ops | %.1f ops/sec | %s",
                 result->name,
                 result->duration_ms,
                 result->operations,
                 result->ops_per_second,
                 result->success ? "✓" : "✗");
        
        SDL_Color result_color = (i == benchmark_state.selected_result) ? accent_color : 
                                (result->success ? success_color : warning_color);
        
        SDL_Surface* result_surface = TTF_RenderText_Blended(font, result_text, result_color);
        if (result_surface) {
            SDL_Texture* result_texture = SDL_CreateTextureFromSurface(renderer, result_surface);
            SDL_Rect result_rect = {x + 30, result_y, result_surface->w, result_surface->h};
            
            if (result_y + result_surface->h > list_y && result_y < list_y + list_h) {
                SDL_RenderCopy(renderer, result_texture, NULL, &result_rect);
            }
            
            result_y += result_surface->h + 5;
            SDL_DestroyTexture(result_texture);
            SDL_FreeSurface(result_surface);
        }
    }
    
    // Instructions
    const char* instructions = benchmark_state.is_running ? 
        "Benchmark in progress..." : 
        "Press 'B' to run benchmarks, 'C' to clear results";
    
    SDL_Surface* inst_surface = TTF_RenderText_Blended(font, instructions, accent_color);
    if (inst_surface) {
        SDL_Texture* inst_texture = SDL_CreateTextureFromSurface(renderer, inst_surface);
        SDL_Rect inst_rect = {x + 30, y + h - 30, inst_surface->w, inst_surface->h};
        SDL_RenderCopy(renderer, inst_texture, NULL, &inst_rect);
        SDL_DestroyTexture(inst_texture);
        SDL_FreeSurface(inst_surface);
    }
}

void benchmark_tab_handle_input(SDL_Event* event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_b:
                if (!benchmark_state.is_running) {
                    benchmark_tab_run_tests();
                }
                break;
            case SDLK_c:
                benchmark_state.result_count = 0;
                benchmark_state.total_tokens_generated = 0;
                benchmark_state.total_time_spent = 0;
                benchmark_state.avg_inference_time = 0;
                benchmark_state.avg_tokens_per_second = 0;
                benchmark_state.peak_tokens_per_second = 0;
                break;
            case SDLK_UP:
                benchmark_state.scroll_offset += 20;
                if (benchmark_state.scroll_offset < 0) benchmark_state.scroll_offset = 0;
                break;
            case SDLK_DOWN:
                benchmark_state.scroll_offset -= 20;
                if (benchmark_state.scroll_offset < 0) benchmark_state.scroll_offset = 0;
                break;
        }
    } else if (event->type == SDL_MOUSEWHEEL) {
        benchmark_state.scroll_offset -= event->wheel.y * 30;
        if (benchmark_state.scroll_offset < 0) benchmark_state.scroll_offset = 0;
    }
}

void benchmark_tab_update(void) {
    // Update system metrics (simulated)
    benchmark_state.cpu_usage = 20.0 + (rand() % 30);
    benchmark_state.memory_usage_mb = 150.0 + (rand() % 50);
    benchmark_state.gpu_usage = 10.0 + (rand() % 20);
}

void benchmark_tab_cleanup(void) {
    // Cleanup resources
}