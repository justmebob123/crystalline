# User Testing Guide - New Features

## Quick Start

### 1. Build and Install
```bash
cd ~/code/math/crystalline
git pull origin main
make clean && make && make app
sudo make install
```

### 2. Run Application
```bash
app/hyper_prime_spiral
```

## What's New

### ✅ Automatic Model Loading
**What it does**: Automatically loads your saved model on startup

**How to test**:
1. Start the application
2. Check terminal output for:
   ```
   ✓ Model loaded successfully!
     Vocabulary size: XXXX
     Embedding dimension: XXX
     Number of layers: X
   ✓ Inference context created
   ```
3. Go to LLM tab - should show "Model loaded and ready"
4. Go to Training tab - model should be ready for training

**If no model exists**: You'll see a message explaining how to create one

### ✅ Crawler URL Input
**What it does**: Lets you specify where the crawler should start

**How to test**:
1. Go to Training tab
2. Scroll down to "Crawler Start URL:" input field
3. Default URL is Wikipedia main page
4. Try changing it to any URL you want to crawl
5. Click START CRAWLER
6. Check terminal - should show your URL

**Example URLs to try**:
- `https://en.wikipedia.org/wiki/Artificial_intelligence`
- `https://news.ycombinator.com`
- `https://www.gutenberg.org`

### ✅ Crawler Status Display
**What it does**: Shows what the crawler is doing in real-time

**How to test**:
1. Go to Training tab
2. Enter a URL in the crawler input
3. Click START CRAWLER
4. Look at the main display area (left side)
5. You should see:
   - "CRAWLER ACTIVE" banner
   - Pages crawled count
   - Queue size
   - Current URL being crawled

**Note**: The crawler thread isn't fully integrated yet, so it won't actually crawl. But the UI and state management are ready.

### ✅ Link Queue Management
**What it does**: Saves crawl queue to a file so you can resume later

**How to test**:
1. Start the crawler
2. Check that `crawler_data/links_to_crawl.txt` was created
3. Open the file - should contain your starting URL
4. Add more URLs manually (one per line)
5. Stop and restart the application
6. Start crawler again - queue size should reflect added URLs

**Manual URL addition**:
```bash
echo "https://example.com" >> crawler_data/links_to_crawl.txt
echo "https://another-site.com" >> crawler_data/links_to_crawl.txt
```

### ✅ UI Improvements
**What changed**: Fixed visual artifact (strange character before "Ready")

**How to test**:
1. Go to Training tab
2. Look at the STATUS section
3. Should see "* Ready" (with asterisk, not weird character)

## Expected Behavior

### Model Loading
- **With saved model**: Loads automatically, shows info, ready to use
- **Without saved model**: Shows instructions, can create new one

### Crawler
- **Before starting**: Shows "START CRAWLER" button (green)
- **After starting**: Shows "STOP CRAWLER" button (red)
- **Status display**: Shows in main area when active
- **Queue file**: Created in `crawler_data/` directory

### Training
- **With model loaded**: Can start training immediately
- **Without model**: Creates new model when training starts

## Troubleshooting

### Model not loading
- Check if `models/saved_model.cllm` exists
- If not, train a model first (Training tab → START TRAINING)

### Crawler not starting
- Make sure you entered a valid URL
- Check terminal for error messages
- Verify `crawler_data/` directory was created

### UI looks wrong
- Try resizing window
- Check terminal for any error messages
- Verify SDL2 and SDL2_ttf are installed

## What Still Needs Work

1. **Crawler Thread**: The START CRAWLER button updates state but doesn't actually start crawling yet. This requires integration with the crawler thread system.

2. **URL Validation**: URLs aren't validated yet - you can enter anything. Invalid URLs will be caught when the crawler actually runs.

3. **Queue Display**: The queue size is shown but individual URLs aren't displayed in the UI yet.

## Feedback Needed

Please test and report:
1. Does the model load automatically?
2. Can you see and edit the crawler URL?
3. Does the crawler status display show up?
4. Is the link queue file created?
5. Any visual glitches or errors?

## Next Steps

Once you confirm everything works:
1. We can integrate the actual crawler thread
2. Add more crawler controls (pause/resume, speed, etc.)
3. Implement URL validation
4. Add queue display UI
5. Add model selector dropdown

---

**All changes committed**: Commit `d407099`
**Ready for testing**: Yes
**Estimated test time**: 10-15 minutes