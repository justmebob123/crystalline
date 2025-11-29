# Crystalline CLLM - Crawler Pipeline Implementation

## 🔴 PHASE 1: CRITICAL FIXES (In Progress)

### 1.1 Preprocessor Fixes ✅ COMPLETED
- [x] Fix infinite loop bug (marker files)
- [x] Fix clean_text() trimming bug
- [x] Add comprehensive debug output
- [x] Build and test

### 1.2 Integrate Prime-Based URL Randomization (HIGH PRIORITY - 2 hours)
- [ ] Modify crawler_get_next_url() to use prime selection
- [ ] Load URLs into memory array
- [ ] Use select_prime_link_index() for selection
- [ ] Remove selected URL from array
- [ ] Add configuration for prime parameters
- [ ] Test randomization works correctly

### 1.3 Add File Type Detection (HIGH PRIORITY - 1 hour)
- [ ] Implement magic byte detection
- [ ] Detect HTML, PDF, images, office docs
- [ ] Route to appropriate processor
- [ ] Skip unsupported binary formats
- [ ] Test with various file types

### 1.4 Improve HTML Tag Removal (MEDIUM PRIORITY - 1 hour)
- [ ] Better script/style tag handling
- [ ] Extract text from common elements (p, div, span, etc.)
- [ ] Handle nested tags correctly
- [ ] Preserve meaningful whitespace
- [ ] Test with real web pages

### 1.5 Add Robots.txt Support (MEDIUM PRIORITY - 1 hour)
- [ ] Download and parse robots.txt
- [ ] Check URL against rules before crawling
- [ ] Respect crawl-delay directive
- [ ] Cache robots.txt per domain
- [ ] Test with various robots.txt files

## 🔵 PHASE 2: ENHANCEMENTS

### 2.1 Per-Domain Rate Limiting
- [ ] Track requests per domain
- [ ] Enforce domain-specific delays
- [ ] Prevent hammering single domain
- [ ] Add domain blacklist/whitelist

### 2.2 Advanced URL Pattern Detection
- [ ] JavaScript onclick handlers
- [ ] Data attributes (data-href, data-url)
- [ ] Meta refresh redirects
- [ ] Sitemap.xml references
- [ ] Canonical URLs

### 2.3 Content Quality Filtering
- [ ] Detect and skip error pages (404, 500, etc.)
- [ ] Detect and skip login/paywall pages
- [ ] Detect and skip duplicate content
- [ ] Detect and skip low-quality content

### 2.4 Improved Tokenization
- [ ] Better handling of special characters
- [ ] Preserve code blocks and technical content
- [ ] Handle multiple languages
- [ ] Preserve mathematical notation

## 📊 CURRENT STATUS

### Completed ✅
- Infinite loop fix
- clean_text() trimming fix
- Debug output added
- Crawl speed improved (5-15s)
- Build successful

### In Progress 🔄
- Prime-based URL randomization
- File type detection
- HTML tag removal improvements

### Pending ⏳
- Robots.txt support
- Per-domain rate limiting
- Advanced URL patterns
- Content quality filtering

## 🎯 IMMEDIATE NEXT STEPS

1. **Integrate Prime Randomization** (2 hours)
   - Modify crawler_get_next_url()
   - Test with real URLs

2. **Add File Type Detection** (1 hour)
   - Magic byte detection
   - Route to processors

3. **Test Complete Pipeline** (1 hour)
   - Run crawler with all fixes
   - Verify no infinite loops
   - Verify randomization
   - Verify file type handling

4. **User Testing & Feedback**
   - Get user to test
   - Collect feedback
   - Iterate on issues

## 📝 NOTES

### Model Persistence
- ✅ ALREADY WORKING - saves to {data_dir}/model.cllm
- ✅ Auto-loads on startup
- ✅ Saves every 5 batches

### Crawl Speed
- ✅ ALREADY WORKING - 5-15s random delays
- ✅ Configurable via crawler_set_rate_limit()

### Prime Randomization
- ✅ Code exists in prime_randomization.c
- ❌ NOT integrated with crawler_core.c
- 🔄 IMPLEMENTING NOW

### File Type Detection
- ❌ NOT implemented
- 🔄 IMPLEMENTING NOW