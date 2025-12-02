#ifndef TAB_TRAINING_H
#define TAB_TRAINING_H

#include "app_common.h"

void init_training_tab(AppState* state);
void cleanup_training_tab(void);
void draw_training_tab(SDL_Renderer* renderer, AppState* state);
void handle_training_tab_click(AppState* state, int x, int y);

#endif /* TAB_TRAINING_H */