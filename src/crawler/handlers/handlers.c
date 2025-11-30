#include "handlers.h"
#include <stdio.h>

/**
 * Register all built-in site handlers
 */
void register_all_handlers(void) {
    printf("Registering built-in site handlers...\n");
    
    // Initialize registry
    site_handlers_init();
    
    // Register Twitter handler
    const SiteHandler* twitter = get_twitter_handler();
    if (twitter) {
        site_handlers_register(twitter);
    }
    
    // Register Britannica handler
    const SiteHandler* britannica = get_britannica_handler();
    if (britannica) {
        site_handlers_register(britannica);
    }
    
    // Register Etymonline handler
    const SiteHandler* etymonline = get_etymonline_handler();
    if (etymonline) {
        site_handlers_register(etymonline);
    }
    
    printf("Site handlers registration complete\n");
}
