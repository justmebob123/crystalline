# Web Crawler + Continuous Training Pipeline Design

## Architecture Overview

```
┌─────────────────┐
│  Web Crawler    │ (Thread 1)
│  - Slow crawl   │
│  - Extract links│
│  - Save pages   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ raw_pages/      │ (Directory)
│  page_001.html  │
│  page_002.html  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Preprocessor    │ (Thread 2)
│  - HTML→Text    │
│  - Clean text   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ preprocessed/   │ (Directory)
│  page_001.txt   │
│  page_002.txt   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Tokenizer       │ (Thread 3)
│  - Tokenize     │
│  - Build vocab  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ training_queue/ │ (Directory)
│  page_001.tok   │
│  page_002.tok   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Training Threads│ (Threads 4-N)
│  - Load file    │
│  - Train model  │
│  - Move to done │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ trained/        │ (Directory)
│  page_001.tok   │
│  page_002.tok   │
└─────────────────┘
```

## Directory Structure

```
~/.cllm_crawler/
├── raw_pages/          # Downloaded HTML pages
├── preprocessed/       # Cleaned text files
├── training_queue/     # Tokenized files ready for training
├── trained/            # Completed training files
├── links_to_crawl.txt  # Queue of URLs to visit
├── links_crawled.txt   # Already visited URLs
├── crawler.lock        # File lock for crawler
└── training.lock       # File lock for training threads
```

## Thread Allocation

- **CPU Count**: `nproc` (e.g., 16 cores)
- **Reserved**: 1 core for main app
- **Crawler**: 1 thread
- **Preprocessor**: 1 thread  
- **Tokenizer**: 1 thread
- **Training**: Remaining threads (e.g., 13 threads)

## File Locking Strategy

### Training Queue
- Each file has a `.lock` companion file
- Thread creates `page_001.tok.lock` before processing
- Other threads skip files with `.lock`
- Lock deleted after successful training
- On crash, stale locks cleaned up on restart

### Crawler State
- `links_to_crawl.txt` - append-only queue
- `links_crawled.txt` - append-only log
- Atomic file operations with temp files + rename

## Crawler Behavior

### Rate Limiting
- **Delay between requests**: 2-5 seconds (random)
- **Max pages per domain**: 100 (configurable)
- **Respect robots.txt**: Yes
- **User-Agent**: Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0

### Link Extraction
- Parse all `<a href="">` tags
- Filter: same domain only (configurable)
- Deduplicate against `links_crawled.txt`
- Add to `links_to_crawl.txt`

### Page Saving
- Save raw HTML to `raw_pages/`
- Filename: `domain_timestamp_hash.html`
- Include metadata: URL, timestamp, status code

## Preprocessing Pipeline

### HTML → Text
1. Remove `<script>`, `<style>`, `<nav>`, `<footer>`
2. Extract text from `<p>`, `<article>`, `<main>`
3. Clean whitespace
4. Remove special characters
5. Normalize unicode

### Text Cleaning
- Remove URLs
- Remove email addresses
- Fix encoding issues
- Normalize whitespace
- Minimum length: 100 characters

## Tokenization

### Vocabulary Building
- Incremental vocabulary updates
- Word-level tokenization
- Special tokens: `<PAD>`, `<UNK>`, `<START>`, `<END>`
- Save vocab to model

### Token File Format
```
# URL: https://example.com/page
# Timestamp: 2024-01-01 12:00:00
# Token count: 1234
token1 token2 token3 ...
```

## Training Integration

### Continuous Training
- Monitor `training_queue/` directory
- Load one file at a time
- Train for N epochs (configurable)
- Update model incrementally
- Save model after each file
- Move file to `trained/`

### Model Persistence
- Save to `~/.cllm_models/continuous_model.cllm`
- Backup every N files
- Checkpoint system

## Status Display

### Console Output
```
=== CRAWLER STATUS ===
Current URL: https://example.com/page
Links in queue: 1,234
Links crawled: 567
Pages downloaded: 567
Pages preprocessed: 560
Pages tokenized: 555
Pages trained: 550

=== TRAINING STATUS ===
Active threads: 13
Current file: page_123.tok
Epoch: 5/10
Loss: 2.3456
Files in queue: 5
Files trained: 550
```

## Command Line Application

### Usage
```bash
# Start crawler + training
cllm_crawler --start-url https://example.com --max-pages 1000

# Resume from checkpoint
cllm_crawler --resume

# Status check
cllm_crawler --status

# Stop gracefully
cllm_crawler --stop
```

### Configuration File
```ini
[crawler]
start_url = https://example.com
max_pages = 1000
delay_min = 2
delay_max = 5
user_agent = Mozilla/5.0...
respect_robots = true

[training]
batch_size = 4
sequence_length = 32
epochs_per_file = 5
learning_rate = 0.001
num_threads = 13

[paths]
data_dir = ~/.cllm_crawler
model_path = ~/.cllm_models/continuous_model.cllm
```

## UI Integration

### New Button: "START CRAWLER"
- Separate from manual training
- Shows crawler status
- Stop button to halt crawler
- Progress indicators for each stage

## Implementation Priority

1. ✅ Directory structure setup
2. ✅ File locking system
3. ✅ Crawler thread (basic)
4. ✅ Preprocessor thread
5. ✅ Tokenizer thread
6. ✅ Training integration
7. ✅ Command line app
8. ✅ UI integration
9. ✅ Status display
10. ✅ Configuration system

## Safety Features

- Graceful shutdown on SIGINT
- Checkpoint recovery
- Stale lock cleanup
- Error logging
- Rate limiting
- Domain blacklist
- Max depth limit
</file_path>