#ifndef TAB_VIDEO_H
#define TAB_VIDEO_H

#include "../../app_common.h"

/**
 * Initialize video tab UI
 */
void init_video_tab(void);

/**
 * Draw video tab
 */
void draw_video_tab(SDL_Renderer* renderer, AppState* state);

/**
 * Handle video tab mouse click
 */
void handle_video_tab_click(AppState* state, int x, int y);

#endif // TAB_VIDEO_H