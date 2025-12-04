#include "label.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void label_render_func(CrystallineElement* element, SDL_Renderer* renderer);
static void label_destroy_func(CrystallineElement* element);

// ============================================================================
// LABEL LIFECYCLE
// ============================================================================

CrystallineLabel* crystalline_label_create(
    CrystallineBounds bounds,
    const char* text,
    TTF_Font* font
) {
    CrystallineLabel* label = (CrystallineLabel*)calloc(1, sizeof(CrystallineLabel));
    if (!label) return NULL;
    
    // Initialize base element
    CrystallineElement* base = &label->base;
    base->type = CRYSTALLINE_ELEMENT_LABEL;
    base->bounds = bounds;
    base->padding = crystalline_padding_uniform(0.0f);
    base->visible = true;
    base->enabled = true;
    base->state = CRYSTALLINE_STATE_NORMAL;
    
    // No background or border by default
    base->bg_color = crystalline_color(0, 0, 0, 0);
    base->border_color = crystalline_color(0, 0, 0, 0);
    base->border_width = 0.0f;
    
    // Set callbacks
    base->render = label_render_func;
    base->destroy = label_destroy_func;
    
    // Initialize label-specific fields
    label->text = text ? strdup(text) : NULL;
    label->font = font;
    label->alignment = CRYSTALLINE_TEXT_ALIGN_LEFT;
    label->text_color = crystalline_color(220, 220, 220, 255);
    
    return label;
}

void crystalline_label_destroy(CrystallineLabel* label) {
    if (!label) return;
    
    if (label->text) {
        free((void*)label->text);
    }
    
    crystalline_element_destroy(&label->base);
}

// ============================================================================
// LABEL CONFIGURATION
// ============================================================================

void crystalline_label_set_text(
    CrystallineLabel* label,
    const char* text
) {
    if (!label) return;
    
    if (label->text) {
        free((void*)label->text);
    }
    
    label->text = text ? strdup(text) : NULL;
}

void crystalline_label_set_font(
    CrystallineLabel* label,
    TTF_Font* font
) {
    if (!label) return;
    label->font = font;
}

void crystalline_label_set_alignment(
    CrystallineLabel* label,
    CrystallineTextAlign alignment
) {
    if (!label) return;
    label->alignment = alignment;
}

void crystalline_label_set_color(
    CrystallineLabel* label,
    CrystallineColor color
) {
    if (!label) return;
    label->text_color = color;
}

// ============================================================================
// RENDERING
// ============================================================================

static void label_render_func(CrystallineElement* element, SDL_Renderer* renderer) {
    CrystallineLabel* label = (CrystallineLabel*)element;
    
    if (!label->text || !label->font) return;
    
    SDL_Color text_color = crystalline_color_to_sdl(label->text_color);
    SDL_Surface* surface = TTF_RenderText_Blended(label->font, label->text, text_color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    // Calculate position based on alignment
    int x, y;
    CrystallinePoint top_left = crystalline_bounds_top_left(element->bounds);
    
    switch (label->alignment) {
        case CRYSTALLINE_TEXT_ALIGN_LEFT:
            x = (int)top_left.x;
            break;
        case CRYSTALLINE_TEXT_ALIGN_CENTER:
            x = (int)(element->bounds.center.x - surface->w / 2);
            break;
        case CRYSTALLINE_TEXT_ALIGN_RIGHT:
            x = (int)(top_left.x + element->bounds.width - surface->w);
            break;
        default:
            x = (int)top_left.x;
    }
    
    y = (int)(element->bounds.center.y - surface->h / 2);
    
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void crystalline_label_render(
    CrystallineLabel* label,
    SDL_Renderer* renderer
) {
    if (!label) return;
    crystalline_element_render(&label->base, renderer);
}

// ============================================================================
// CLEANUP
// ============================================================================

static void label_destroy_func(CrystallineElement* element) {
    CrystallineLabel* label = (CrystallineLabel*)element;
    
    if (label->text) {
        free((void*)label->text);
        label->text = NULL;
    }
}