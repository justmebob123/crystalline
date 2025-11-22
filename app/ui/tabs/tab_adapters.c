/*
 * Tab Adapters - Bridge between new tab system and existing AppState
 */

#include "../../app_common.h"
#include <SDL2/SDL_ttf.h>

// Forward declarations of actual tab rendering functions
extern void draw_llm_tab(SDL_Renderer* renderer, AppState* state);
extern void draw_training_tab(SDL_Renderer* renderer, AppState* state);
extern void draw_research_tab(SDL_Renderer* renderer, AppState* state);

// Adapter for LLM tab
void llm_tab_render_adapter(SDL_Renderer* renderer, AppState* state) {
    draw_llm_tab(renderer, state);
}

// Adapter for Training tab
void training_tab_render_adapter(SDL_Renderer* renderer, AppState* state) {
    draw_training_tab(renderer, state);
}

// Adapter for Research tab
void research_tab_render_adapter(SDL_Renderer* renderer, AppState* state) {
    draw_research_tab(renderer, state);
}