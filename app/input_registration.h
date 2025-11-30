// app/input_registration.h - Centralized Input Registration System
#ifndef INPUT_REGISTRATION_H
#define INPUT_REGISTRATION_H

#include "input_manager.h"

/**
 * Initialize all inputs for all tabs at startup.
 * This ensures inputs are available immediately, regardless of which tab is visited first.
 * 
 * CRITICAL: This function MUST be called during app initialization, before the main loop.
 * 
 * @param manager The global InputManager instance
 */
void init_all_inputs(InputManager* manager);

#endif // INPUT_REGISTRATION_H