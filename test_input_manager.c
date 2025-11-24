#include "app/input_manager.h"
#include <stdio.h>
#include <assert.h>

int main() {
    printf("Testing InputManager...\n");
    
    // Create manager
    InputManager* manager = input_manager_create();
    assert(manager != NULL);
    printf("✓ Created InputManager\n");
    
    // Register inputs
    ManagedInput* input1 = input_manager_register(manager, "test.input1", 0, INPUT_TYPE_TEXT,
                                                  (SDL_Rect){100, 100, 200, 30});
    assert(input1 != NULL);
    printf("✓ Registered input1\n");
    
    ManagedInput* input2 = input_manager_register(manager, "test.input2", 0, INPUT_TYPE_URL,
                                                  (SDL_Rect){100, 150, 200, 30});
    assert(input2 != NULL);
    printf("✓ Registered input2\n");
    
    // Set text
    input_manager_set_text(manager, "test.input1", "Hello World");
    const char* text1 = input_manager_get_text(manager, "test.input1");
    assert(strcmp(text1, "Hello World") == 0);
    printf("✓ Set/Get text works: '%s'\n", text1);
    
    // Focus management
    input_manager_focus(manager, "test.input1");
    assert(manager->focused_input == input1);
    assert(input1->active == true);
    printf("✓ Focus works\n");
    
    input_manager_unfocus(manager);
    assert(manager->focused_input == NULL);
    assert(input1->active == false);
    printf("✓ Unfocus works\n");
    
    // Cleanup
    input_manager_destroy(manager);
    printf("✓ Destroyed InputManager\n");
    
    printf("\n✅ ALL TESTS PASSED\n");
    return 0;
}