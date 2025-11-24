# Crawler Library API Design

## Problem Statement
Currently, the application shells out to the CLI tool instead of using the library directly. This is architecturally wrong because:
1. The CLI tool and app should be independent consumers of the library
2. They might not even exist on the same platform
3. The app should have direct access to crawler state, not read log files
4. The library should provide thread-safe state access

## Correct Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      libcrawler.so                          │
│  (Pure library - no CLI, no UI, just functionality)         │
│                                                              │
│  - crawler_init()                                           │
│  - crawler_start()                                          │
│  - crawler_stop()                                           │
│  - crawler_get_status()                                     │
│  - crawler_set_callback()                                   │
│  - crawler_cleanup()                                        │
└─────────────────────────────────────────────────────────────┘
                    ▲                        ▲
                    │                        │
                    │                        │
        ┌───────────┴──────────┐   ┌────────┴──────────┐
        │   cllm_crawler       │   │    app (GUI)      │
        │   (CLI tool)         │   │                   │
        │                      │   │                   │
        │  - Parses args       │   │  - SDL2 UI        │
        │  - Calls library     │   │  - Calls library  │
        │  - Prints to stdout  │   │  - Updates UI     │
        └──────────────────────┘   └───────────────────┘
```

## Required Library API

### State Management
```c
typedef struct CrawlerState CrawlerState;

// Initialize crawler
CrawlerState* crawler_init(const char* data_dir, const char* start_url, int max_pages);

// Start crawler (spawns threads internally)
int crawler_start(CrawlerState* state);

// Stop crawler (stops threads)
void crawler_stop(CrawlerState* state);

// Cleanup
void crawler_cleanup(CrawlerState* state);
```

### Status Query (Thread-Safe)
```c
typedef struct {
    int running;
    int pages_crawled;
    int pages_in_queue;
    int pages_preprocessed;
    int pages_tokenized;
    int pages_trained;
    char current_url[512];
    char last_error[512];
} CrawlerStatus;

// Get current status (thread-safe)
void crawler_get_status(CrawlerState* state, CrawlerStatus* status);
```

### Callback System
```c
typedef enum {
    CRAWLER_EVENT_PAGE_DOWNLOADED,
    CRAWLER_EVENT_PAGE_PREPROCESSED,
    CRAWLER_EVENT_PAGE_TOKENIZED,
    CRAWLER_EVENT_PAGE_TRAINED,
    CRAWLER_EVENT_ERROR,
    CRAWLER_EVENT_STOPPED
} CrawlerEventType;

typedef struct {
    CrawlerEventType type;
    char message[512];
    int pages_crawled;
} CrawlerEvent;

typedef void (*CrawlerCallback)(const CrawlerEvent* event, void* user_data);

// Set callback for events
void crawler_set_callback(CrawlerState* state, CrawlerCallback callback, void* user_data);
```

## Implementation Plan

1. **Enhance crawler_core.c**
   - Add `crawler_start()` function that spawns all threads
   - Add `crawler_stop()` function that stops all threads
   - Add `crawler_get_status()` with proper locking
   - Add callback system

2. **Reimplement CLI tool**
   - Remove all crawler logic
   - Just call library functions
   - Print status updates from callbacks

3. **Reimplement app**
   - Remove all `system()` calls
   - Call library directly
   - Update UI from callbacks
   - No more log file reading

4. **Benefits**
   - CLI and app are completely independent
   - Both use the same tested library code
   - No coupling between CLI and app
   - Can run on different platforms
   - Direct state access, no file parsing