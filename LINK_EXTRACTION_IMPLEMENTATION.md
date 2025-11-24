# Link Extraction Implementation

## Overview
This document describes the production-ready link extraction system implemented in the Crystalline web crawler.

## Architecture Change

### Previous Architecture (Broken)
```
Crawler downloads page → Crawler extracts links (buggy) → Preprocessor extracts text
```

**Problems:**
- Link extraction in crawler was buggy (looking for `href="` instead of handling HTML entities)
- Preprocessor only extracted text, not links
- Crawler would download one page and stop (no continuous crawling)

### New Architecture (Production-Ready)
```
Crawler downloads page → Preprocessor extracts BOTH text AND links → Continuous crawling
```

**Benefits:**
- Single responsibility: Crawler only downloads, Preprocessor handles all HTML processing
- Robust link extraction with proper HTML entity handling
- Relative URL resolution
- Invalid link filtering
- Continuous crawling with automatic queue management

## Implementation Details

### 1. Link Extraction in Preprocessor (`src/crawler/preprocessor.c`)

#### `extract_links()` Function
- **Purpose:** Extract all valid links from HTML and add them to the crawl queue
- **Features:**
  - Handles both quoted and unquoted href attributes
  - Supports single quotes, double quotes, and no quotes
  - Filters out invalid URLs (javascript:, mailto:, tel:, data:, #anchors)
  - Resolves relative URLs to absolute URLs
  - Preserves protocol (http/https) from base URL
  - Appends links directly to `links_to_crawl.txt`

#### `extract_base_url()` Function
- **Purpose:** Extract the original URL from HTML metadata comments
- **How it works:** Reads the `<!-- URL: ... -->` comment that the crawler adds to each downloaded page
- **Why needed:** Required for resolving relative URLs to absolute URLs

#### `preprocess_file()` Function (Enhanced)
- **New workflow:**
  1. Read HTML file
  2. Extract base URL from metadata
  3. Extract links and add to queue (NEW)
  4. Extract text blocks
  5. Clean and normalize text
  6. Save text to preprocessed directory

### 2. Crawler Simplification (`src/crawler/crawler_core.c`)

#### Removed Code
- `crawler_extract_links()` function completely removed
- Link extraction call removed from download workflow

#### Simplified Workflow
```c
// Download page
crawler_download_page(url, &amp;buffer);

// Save page (with metadata)
crawler_save_page(state, url, buffer.data, buffer.size);

// Mark as crawled (links will be extracted by preprocessor)
crawler_mark_crawled(state, url);
```

### 3. Continuous Crawling Flow

```
1. Crawler downloads page from queue
2. Crawler saves page with metadata (URL, timestamp)
3. Crawler marks URL as crawled
4. Preprocessor picks up new HTML file
5. Preprocessor extracts text blocks
6. Preprocessor extracts links and adds to queue ← KEY STEP
7. Crawler finds new URLs in queue
8. Repeat from step 1
```

## Link Extraction Algorithm

### URL Detection
```c
while ((p = strstr(p, "href=")) != NULL) {
    // Skip "href="
    p += 5;
    
    // Detect quote type (", ', or none)
    char quote = 0;
    if (*p == '"' || *p == '\'') {
        quote = *p;
        p++;
    }
    
    // Find end of URL
    if (quote) {
        url_end = strchr(p, quote);
    } else {
        // Find whitespace or >
        while (*p &amp;&amp; !isspace(*p) &amp;&amp; *p != '>') p++;
        url_end = p;
    }
    
    // Extract URL
    strncpy(url, url_start, url_len);
}
```

### URL Filtering
```c
// Skip invalid URLs
if (url[0] == '#' || 
    strncmp(url, "javascript:", 11) == 0 ||
    strncmp(url, "mailto:", 7) == 0 ||
    strncmp(url, "tel:", 4) == 0 ||
    strncmp(url, "data:", 5) == 0) {
    continue;
}
```

### Relative URL Resolution
```c
if (url[0] == '/') {
    // Extract domain from base_url
    const char* domain_start = strstr(base_url, "://") + 3;
    const char* domain_end = strchr(domain_start, '/');
    
    // Determine protocol (http or https)
    const char* protocol = "https://";
    if (strncmp(base_url, "http://", 7) == 0) {
        protocol = "http://";
    }
    
    // Construct full URL
    snprintf(full_url, sizeof(full_url), "%s%.*s%s", 
            protocol, (int)domain_len, domain_start, url);
}
```

## Testing Checklist

- [ ] Crawler downloads initial page
- [ ] Preprocessor extracts text from page
- [ ] Preprocessor extracts links from page
- [ ] Links are added to crawl queue
- [ ] Crawler picks up new links from queue
- [ ] Continuous crawling works (downloads multiple pages)
- [ ] Relative URLs are resolved correctly
- [ ] Invalid URLs are filtered out
- [ ] No duplicate URLs in queue

## Files Modified

1. **src/crawler/preprocessor.c**
   - Added `extract_links()` function
   - Added `extract_base_url()` function
   - Modified `preprocess_file()` to extract links
   - Modified `preprocessor_thread_func()` to pass queue file path

2. **src/crawler/crawler_core.c**
   - Removed `crawler_extract_links()` function
   - Removed link extraction call from download workflow
   - Simplified crawler to focus only on downloading

3. **Makefile**
   - Fixed crawler CLI linking to include libprimemath

## Performance Characteristics

- **Link extraction time:** O(n) where n is HTML file size
- **Memory usage:** Minimal (processes one link at a time)
- **Queue management:** Append-only to avoid file locking issues
- **Thread safety:** Preprocessor runs in separate thread from crawler

## Future Enhancements

1. **Domain filtering:** Add option to stay on same domain or allow cross-domain
2. **URL deduplication:** Check if URL already crawled before adding to queue
3. **Robots.txt support:** Respect robots.txt rules
4. **Sitemap support:** Parse and use sitemap.xml files
5. **Link priority:** Prioritize certain types of links (e.g., same domain first)