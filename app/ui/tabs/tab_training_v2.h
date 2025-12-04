#ifndef TAB_TRAINING_V2_H
#define TAB_TRAINING_V2_H

#include "../../app_state.h"
#include <SDL2/SDL.h>

/**
 * Initialize Training Tab V2 (Crystalline UI V2)
 */
void init_training_tab_v2(AppState* state);

/**
 * Draw Training Tab V2
 */
void draw_training_tab_v2(SDL_Renderer* renderer, AppState* state);

/**
 * Handle events for Training Tab V2
 */
void handle_training_tab_v2_event(AppState* state, SDL_Event* event);

/**
 * Cleanup Training Tab V2
 */
void cleanup_training_tab_v2(void);

#endif // TAB_TRAINING_V2_H