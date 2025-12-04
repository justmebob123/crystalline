/**
 * Crystalline UI V2 - Test Demo
 * 
 * This demonstrates the new UI system with automatic layout,
 * unified event handling, and all widgets working together.
 */

#include "core.h"
#include "element.h"
#include "container.h"
#include "button.h"
#include "list.h"
#include "slider.h"
#include "dropdown.h"
#include "panel.h"
#include "label.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

// ============================================================================
// CALLBACKS
// ============================================================================

static void on_button_clicked(void* user_data) {
    const char* label = (const char*)user_data;
    printf("Button clicked: %s\n", label);
}

static void on_slider_changed(float value, void* user_data) {
    const char* name = (const char*)user_data;
    printf("Slider changed: %s = %.1f\n", name, value);
}

static void on_dropdown_selected(int index, const char* option, void* user_data) {
    printf("Dropdown selected: index=%d, option=%s\n", index, option);
    (void)user_data;
}

static void on_list_item_selected(int index, void* user_data) {
    printf("List item selected: index=%d\n", index);
    (void)user_data;
}

static void on_list_item_checked(int index, bool checked, void* user_data) {
    printf("List item checked: index=%d, checked=%d\n", index, checked);
    (void)user_data;
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    if (TTF_Init() < 0) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Crystalline UI V2 Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1200, 800,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Load font
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 14);
    if (!font) {
        printf("TTF_OpenFont failed: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    printf("\n=== Crystalline UI V2 Test Demo ===\n\n");
    
    // ========================================================================
    // CREATE UI HIERARCHY
    // ========================================================================
    
    // Create main container (vertical layout)
    CrystallineContainer* main_container = crystalline_container_create(
        crystalline_bounds(600, 400, 400, 700),
        CRYSTALLINE_LAYOUT_VERTICAL,
        15.0f  // spacing between elements
    );
    main_container->base.padding = crystalline_padding_uniform(20.0f);
    
    // Create title label
    CrystallineLabel* title = crystalline_label_create(
        crystalline_bounds(0, 0, 360, 30),
        "Crystalline UI V2 Demo",
        font
    );
    crystalline_label_set_alignment(title, CRYSTALLINE_TEXT_ALIGN_CENTER);
    title->text_color = crystalline_color(150, 200, 255, 255);
    crystalline_container_add_child(main_container, (CrystallineElement*)title);
    
    // Create dropdown
    CrystallineDropdown* dropdown = crystalline_dropdown_create(
        crystalline_bounds(0, 0, 360, 35),
        on_dropdown_selected,
        NULL
    );
    crystalline_dropdown_set_font(dropdown, font);
    const char* model_options[] = {"Model A", "Model B", "Model C"};
    crystalline_dropdown_set_options(dropdown, model_options, 3);
    crystalline_container_add_child(main_container, (CrystallineElement*)dropdown);
    
    // Create slider 1
    CrystallineSlider* slider1 = crystalline_slider_create(
        crystalline_bounds(0, 0, 360, 30),
        0.0f, 100.0f, 50.0f,
        on_slider_changed,
        (void*)"Slider 1"
    );
    crystalline_slider_set_font(slider1, font);
    crystalline_slider_set_show_value(slider1, true);
    crystalline_container_add_child(main_container, (CrystallineElement*)slider1);
    
    // Create slider 2
    CrystallineSlider* slider2 = crystalline_slider_create(
        crystalline_bounds(0, 0, 360, 30),
        0.0f, 100.0f, 75.0f,
        on_slider_changed,
        (void*)"Slider 2"
    );
    crystalline_slider_set_font(slider2, font);
    crystalline_slider_set_show_value(slider2, true);
    crystalline_container_add_child(main_container, (CrystallineElement*)slider2);
    
    // Create button row (horizontal layout)
    CrystallineContainer* button_row = crystalline_container_create(
        crystalline_bounds(0, 0, 360, 50),
        CRYSTALLINE_LAYOUT_HORIZONTAL,
        10.0f
    );
    button_row->base.bg_color = crystalline_color(0, 0, 0, 0);  // Transparent
    button_row->base.border_width = 0.0f;
    
    CrystallineButton* btn1 = crystalline_button_create(
        "START", CRYSTALLINE_SIZE_MEDIUM, true, on_button_clicked, (void*)"START"
    );
    crystalline_button_set_font(btn1, font);
    crystalline_container_add_child(button_row, (CrystallineElement*)btn1);
    
    CrystallineButton* btn2 = crystalline_button_create(
        "STOP", CRYSTALLINE_SIZE_MEDIUM, true, on_button_clicked, (void*)"STOP"
    );
    crystalline_button_set_font(btn2, font);
    crystalline_container_add_child(button_row, (CrystallineElement*)btn2);
    
    CrystallineButton* btn3 = crystalline_button_create(
        "SAVE", CRYSTALLINE_SIZE_SMALL, true, on_button_clicked, (void*)"SAVE"
    );
    crystalline_button_set_font(btn3, font);
    crystalline_container_add_child(button_row, (CrystallineElement*)btn3);
    
    crystalline_container_add_child(main_container, (CrystallineElement*)button_row);
    
    // Create list label
    CrystallineLabel* list_label = crystalline_label_create(
        crystalline_bounds(0, 0, 360, 20),
        "Files (5):",
        font
    );
    crystalline_container_add_child(main_container, (CrystallineElement*)list_label);
    
    // Create file list
    CrystallineList* file_list = crystalline_list_create(
        crystalline_bounds(0, 0, 360, 200),
        25.0f,
        true  // show checkboxes
    );
    crystalline_list_set_font(file_list, font);
    const char* files[] = {
        "training_data_1.txt",
        "training_data_2.txt",
        "training_data_3.txt",
        "training_data_4.txt",
        "training_data_5.txt"
    };
    crystalline_list_set_items(file_list, files, 5);
    crystalline_list_set_select_callback(file_list, on_list_item_selected, NULL);
    crystalline_list_set_check_callback(file_list, on_list_item_checked, NULL);
    crystalline_container_add_child(main_container, (CrystallineElement*)file_list);
    
    // Compute layout - this positions everything automatically!
    printf("Computing layout...\n");
    crystalline_container_compute_layout(main_container);
    printf("✓ Layout computed\n\n");
    
    // Print element positions
    printf("Element positions after layout:\n");
    for (int i = 0; i < main_container->base.child_count; i++) {
        CrystallineElement* child = main_container->base.children[i];
        printf("  Child %d: center=(%.1f, %.1f) size=(%.1fx%.1f)\n",
               i, child->bounds.center.x, child->bounds.center.y,
               child->bounds.width, child->bounds.height);
    }
    printf("\n");
    
    // ========================================================================
    // MAIN LOOP
    // ========================================================================
    
    bool running = true;
    SDL_Event event;
    
    printf("Demo running. Press ESC to quit.\n\n");
    
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            } else {
                // Route event to UI - automatic routing!
                crystalline_container_handle_event(main_container, &event);
            }
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
        SDL_RenderClear(renderer);
        
        // Render UI - automatic rendering!
        crystalline_container_render(main_container, renderer);
        
        // Present
        SDL_RenderPresent(renderer);
        
        SDL_Delay(16);  // ~60 FPS
    }
    
    // ========================================================================
    // CLEANUP
    // ========================================================================
    
    printf("\nCleaning up...\n");
    crystalline_container_destroy(main_container);
    
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    printf("✓ Demo complete\n");
    
    return 0;
}