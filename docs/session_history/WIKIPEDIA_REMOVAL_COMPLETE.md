# Wikipedia Default URL Removal - COMPLETE

## Problem
The application was using hardcoded Wikipedia URLs as defaults in 3 locations, causing the crawler to ignore user-provided URLs and always crawl Wikipedia instead.

## Root Cause
Even though the user pasted `https://x.com/JustMeBob123`, the system was using the default Wikipedia URL because:

1. **app/main.c** - State initialized with Wikipedia URL
2. **app/ui/tabs/tab_training.c** - Input field initialized with Wikipedia URL  
3. **tools/cllm_crawler.c** - Crawler tool had Wikipedia as fallback

## Solution Applied

### Files Modified (3)

#### 1. app/ui/tabs/tab_training.c (line ~127)
```c
// BEFORE:
text_input_set_text(&crawler_url_input, "https://en.wikipedia.org/wiki/Main_Page");

// AFTER:
text_input_set_text(&crawler_url_input, ""); // User must provide URL
```

#### 2. app/main.c (line ~154)
```c
// BEFORE:
strcpy(state->crawler_start_url, "https://en.wikipedia.org/wiki/Main_Page");

// AFTER:
state->crawler_start_url[0] = '\0'; // No default URL - user must provide
```

#### 3. tools/cllm_crawler.c (line ~47)
```c
// BEFORE:
char start_url[2048] = "https://en.wikipedia.org/wiki/Main_Page";

// AFTER:
char start_url[2048] = ""; // No default - must be provided via --start-url
```

## Verification

Confirmed NO Wikipedia references remain:
```bash
$ grep -r "wikipedia" --include="*.c" --include="*.h" -i
# (no results)
```

## Expected Behavior Now

1. **Input field starts empty** - No default URL
2. **User MUST paste/type URL** - Required before crawling
3. **Crawler uses provided URL** - No fallback to Wikipedia
4. **Error if empty** - "Please enter a valid URL" if user clicks START CRAWLER without providing URL

## Testing Instructions

```bash
cd ~/code/math/crystalline
git pull origin main
make clean && make && make app
app/hyper_prime_spiral
```

Then:
1. Go to Training tab
2. Paste your URL (e.g., https://x.com/JustMeBob123)
3. Click START CRAWLER
4. Verify terminal shows YOUR URL, not Wikipedia

## Commit
- **Commit**: 46bccd4
- **Message**: "CRITICAL FIX: Remove ALL Wikipedia default URLs - user must provide URL"
- **Status**: Ready to push (awaiting user's `git push`)

## Summary
✅ All 3 Wikipedia references removed
✅ Clean build successful
✅ User must now explicitly provide URL
✅ No more automatic Wikipedia crawling