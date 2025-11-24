# Quick Start Guide - Web Crawler System

## Installation

```bash
cd ~/code/math/crystalline
git pull origin main
make clean && make && make crawler
sudo make install
```

## Running the Crawler

### Basic Usage
```bash
# Start crawling Wikipedia
tools/cllm_crawler --start-url https://en.wikipedia.org/wiki/Main_Page --max-pages 100
```

### Custom Configuration
```bash
# Crawl specific site with more threads
tools/cllm_crawler \
  --start-url https://example.com \
  --max-pages 500 \
  --threads 12 \
  --data-dir ~/my_crawler_data \
  --model-path ~/my_models/custom_model.cllm
```

## What Happens

1. **Crawler downloads pages** (slow, 2-5 sec delays)
   - Saves to `~/.cllm_crawler/raw_pages/`
   - Extracts links for future crawling

2. **Preprocessor cleans HTML** (fast)
   - Removes tags, scripts, styles
   - Saves to `~/.cllm_crawler/preprocessed/`

3. **Tokenizer processes text** (fast)
   - Word-level tokenization
   - Saves to `~/.cllm_crawler/training_queue/`

4. **Training threads process files** (CPU-intensive)
   - Multiple threads work in parallel
   - Each file trained for 5 epochs
   - Model saved after each file
   - Files moved to `~/.cllm_crawler/trained/`

## Monitoring

Watch the console output:
```
=== Crawling [1/100] ===
URL: https://example.com
✓ Saved: page_12345_1234567890.html
  Found 25 links
Waiting 3 seconds...

Preprocessing: page_12345_1234567890.html
✓ Preprocessed: page_12345_1234567890

Tokenizing: page_12345_1234567890.txt
✓ Tokenized: page_12345_1234567890 (1234 tokens)

=== Training on file ===
File: page_12345_1234567890.tok
Loaded 1234 tokens
  Epoch 1/5: loss = 9.4863
  Epoch 2/5: loss = 9.4629
  ...
✓ Training complete: avg loss = 9.4174
✓ Model saved
```

## Stopping

Press `Ctrl+C` for graceful shutdown:
- Crawler stops after current page
- Preprocessor finishes current file
- Tokenizer finishes current file
- Training threads complete current file
- Model saved
- All threads joined

## Resuming

Just run the command again:
- Reads `links_to_crawl.txt` queue
- Skips already processed files
- Continues from where it left off

## Using the Trained Model

### In UI Application
```bash
# Copy model to UI directory
cp ~/.cllm_models/continuous_model.cllm models/saved_model.cllm

# Start UI
app/hyper_prime_spiral

# Go to LLM tab and generate text
```

### In Command Line
The model is automatically saved to `~/.cllm_models/continuous_model.cllm` and can be loaded by any CLLM application.

## Performance

### Crawling Speed
- ~10-20 pages/minute (intentionally slow)
- Respectful to servers

### Processing Speed
- Preprocessor: ~100 pages/minute
- Tokenizer: ~100 pages/minute
- Training: Depends on file size

### Bottleneck
Training is the slowest part, which is intentional - the crawler won't overwhelm the training system.

## Safety Features

- ✅ Rate limiting (won't hammer servers)
- ✅ File locking (no duplicate processing)
- ✅ Crash recovery (can resume)
- ✅ Graceful shutdown (Ctrl+C)
- ✅ Model persistence (saves after each file)

## Troubleshooting

### "Failed to download"
- Check internet connection
- Check URL is valid
- Some sites block automated access

### "Not enough tokens for batch"
- File too small for training
- Will be skipped automatically
- Increase file size or reduce batch size

### "Failed to create model"
- Check disk space
- Check permissions on model directory

## Summary

**Complete continuous learning system is operational.** Run the crawler, let it work, and the model will continuously improve as it processes more web pages. The system is designed to run indefinitely with proper resource management and crash recovery.
</file_path>