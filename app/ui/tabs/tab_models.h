// tab_models.h - Model Management Tab Header
#ifndef TAB_MODELS_H
#define TAB_MODELS_H

#include "../../app_common.h"

// Initialize models tab
void init_models_tab(AppState* state);

// Cleanup models tab
void cleanup_models_tab(AppState* state);

// Draw models tab
void draw_models_tab(AppState* state);

// Handle click events
void handle_models_tab_click(AppState* state, int x, int y);

#endif // TAB_MODELS_H