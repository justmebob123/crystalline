// app/ui/tabs/tab_crawler.h - Crawler Tab Interface
#ifndef TAB_CRAWLER_H
#define TAB_CRAWLER_H

#include "../../app_common.h"

/**
 * Draw the crawler control tab
 */
void draw_crawler_tab(AppState* state);

/**
 * Handle mouse clicks in the crawler tab
 */
void handle_crawler_tab_click(AppState* state, int mouse_x, int mouse_y);

/**
 * Handle keyboard input in the crawler tab
 */
void handle_crawler_tab_keyboard(AppState* state, int key);

/**
 * Cleanup crawler tab resources
 */
void cleanup_crawler_tab(void);

#endif // TAB_CRAWLER_H