# Comprehensive Tab Audit Plan

## Tabs to Audit

Based on the file listing, we have 9 tabs total:

1. **tab_benchmark.c** - Has original backup (tab_benchmark_original.c.bak)
2. **tab_crawler.c** - No backup found
3. **tab_downloaded_files.c** - Has original backup (tab_downloaded_files_original.c.bak)
4. **tab_llm.c** - No backup found
5. **tab_models.c** - No backup found
6. **tab_research.c** - No backup found (but we know it was modified)
7. **tab_training.c** - Has multiple backups (tab_training.c.backup, tab_training.c.old_broken)
8. **tab_url_manager.c** - No backup found (but we know it was modified)
9. **tab_video.c** - Has original backup (tab_video_original.c.bak)

## Audit Process

For each tab, we will:

1. **Identify Original Functionality**
   - Check if backup exists
   - If no backup, use git to find original version
   - Document all features present in original

2. **Compare with Current Implementation**
   - Check if all features are present
   - Identify missing functionality
   - Identify UI library limitations

3. **Identify Required UI Library Enhancements**
   - List missing UI elements
   - List missing interactions
   - List missing visual features

4. **Implement Missing Features**
   - Add to UI library if needed
   - Update tab to use new features
   - Verify functionality matches original

## Priority Order

Based on conversation history, these tabs were modified:
1. ✅ URL Manager - Already fixed (dodecagon → rectangle)
2. ✅ Research Tab - Already fixed (8 UX issues)
3. ⚠️ Training Tab - Needs complete audit (user reported missing features)
4. ❓ Benchmark Tab - Need to check
5. ❓ Downloaded Files Tab - Need to check
6. ❓ Video Tab - Need to check
7. ❓ Models Tab - Need to check
8. ❓ LLM Tab - Need to check
9. ❓ Crawler Tab - Need to check

## Next Steps

1. Start with Training Tab (highest priority - user feedback)
2. Then audit all other tabs systematically
3. Document findings
4. Implement missing features
5. Verify all functionality restored
