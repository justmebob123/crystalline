# Phase 1: Critical Bug Fixes - Implementation Tracker

## Task 1: Fix Training Thread (30 min) ✅ IN PROGRESS
- [ ] Modify crawler_api.c to add training thread support
- [ ] Add training_threads array to CrawlerState
- [ ] Add training_internal pointer to CrawlerState
- [ ] Initialize continuous_training in crawler_start()
- [ ] Start training worker threads
- [ ] Stop training threads in crawler_stop()
- [ ] Test end-to-end pipeline

## Task 2: Add Multi-Threading (1 hour)
- [ ] Make thread count configurable
- [ ] Add thread_count parameter to API
- [ ] Spawn multiple preprocessor threads
- [ ] Spawn multiple tokenizer threads
- [ ] Spawn multiple training threads (4-8)
- [ ] Test with different thread counts

## Task 3: Remove Embedding Dimension Limit (1 hour)
- [ ] Remove MAX_EMBEDDING_DIM constant
- [ ] Make embedding arrays dynamic (malloc)
- [ ] Update all structs to use pointers
- [ ] Update file I/O to handle variable dimensions
- [ ] Test with 1024, 2048, 4096 dimensions

## Task 4: Remove Vocabulary Size Limit (1 hour)
- [ ] Remove MAX_VOCAB_SIZE constant
- [ ] Make vocabulary fully dynamic
- [ ] Update token arrays to use malloc
- [ ] Support vocabulary growth during training
- [ ] Test with 50K, 100K, 250K tokens

## Task 5: Fix Font Path (30 min)
- [ ] Add multi-path font discovery
- [ ] Check Ubuntu, CentOS, Debian paths
- [ ] Add environment variable override
- [ ] Test on both Ubuntu and CentOS

## Estimated Total: 4-6 hours