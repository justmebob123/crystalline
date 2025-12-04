#include "panel.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void panel_render_func(CrystallineElement* element, SDL_Renderer* renderer);
static void panel_destroy_func(CrystallineElement* element);

// ============================================================================
// PANEL LIFECYCLE
// ============================================================================

CrystallinePanel* crystalline_panel_create(
    CrystallineBounds bounds,
    const char* title
) {
    CrystallinePanel* panel = (CrystallinePanel*)calloc(1, sizeof(CrystallinePanel));
    if (!panel) return NULL;
    
    // Initialize base element
    CrystallineElement* base = &panel->base;
    base->type = CRYSTALLINE_ELEMENT_PANEL;
    base->bounds = bounds;
    base->padding = crystalline_padding_uniform(10.0f);
    base->visible = true;
    base->enabled = true;
    base->state = CRYSTALLINE_STATE_NORMAL;
    
    // Set colors
    base->bg_color = crystalline_color(25, 25, 35, 255);
    base->border_color = crystalline_color(80, 80, 100, 255);
    base->border_width = 2.0f;
    
    // Set callbacks
    base->render = panel_render_func;
    base->destroy = panel_destroy_func;
    
    // Initialize panel-specific fields
    panel->title = title ? strdup(title) : NULL;
    panel->font = NULL;
    panel->show_title = (title != NULL);
    panel->title_color = crystalline_color(150, 150, 170, 255);
    
    return panel;
}

void crystalline_panel_destroy(CrystallinePanel* panel) {
    if (!panel) return;
    
    if (panel->title) {
        free((void*)panel->title);
    }
    
    crystalline_element_destroy(&panel->base);
}

// ============================================================================
// PANEL CONFIGURATION
// ============================================================================

void crystalline_panel_set_title(
    CrystallinePanel* panel,
    const char* title
) {
    if (!panel) return;
    
    if (panel->title) {
        free((void*)panel->title);
    }
    
    panel->title = title ? strdup(title) : NULL;
}

void crystalline_panel_set_show_title(
    CrystallinePanel* panel,
    bool show
) {
    if (!panel) return;
    panel->show_title = show;
}

void crystalline_panel_set_font(
    CrystallinePanel* panel,
    TTF_Font* font
) {
    if (!panel) return;
    panel->font = font;
}

// ============================================================================
// RENDERING
// ============================================================================

static void panel_render_func(CrystallineElement* element, SDL_Renderer* renderer) {
    CrystallinePanel* panel = (CrystallinePanel*)element;
    
    // Draw title if enabled
    if (panel->show_title && panel->title && panel->font) {
        CrystallinePoint top_left = crystalline_bounds_top_left(element->bounds);
        
        SDL_Color title_color = crystalline_color_to_sdl(panel->title_color);
        SDL_Surface* surface = TTF_RenderText_Blended(panel->font, panel->title, title_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect dest = {
                    (int)(top_left.x + 10),
                    (int)(top_left.y + 5),
                    surface->w,
                    surface->h
                };
                SDL_RenderCopy(renderer, texture, NULL, &dest);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
}

void crystalline_panel_render(
    CrystallinePanel* panel,
    SDL_Renderer* renderer
) {
    if (!panel) return;
    crystalline_element_render(&panel->base, renderer);
}

// ============================================================================
// CLEANUP
// ============================================================================

static void panel_destroy_func(CrystallineElement* element) {
    CrystallinePanel* panel = (CrystallinePanel*)element;
    
    if (panel->title) {
        free((void*)panel->title);
        panel->title = NULL;
    }
}