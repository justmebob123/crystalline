# Real-Time Sphere Stats Update Plan

## Problem
Sphere visualization stats only update AFTER each epoch completes, not during training.
Users see static spheres during long epochs.

## Root Cause
- `threaded_train_epoch_lockfree()` is a blocking call
- `update_sphere_stats()` only called after epoch completes
- No continuous polling during epoch execution

## Solution: Background Stats Update Thread

### Approach
Add a dedicated stats update thread that runs during training:

1. **Stats Update Thread**
   - Runs in background during training
   - Polls sphere stats every 100ms
   - Updates AppState->sphere_stats
   - Thread-safe with mutex protection

2. **Integration Points**
   - Start thread when training begins
   - Stop thread when training ends
   - Works for both training tab and crawler

3. **Implementation**
   ```c
   // In training_thread.c
   static pthread_t stats_update_thread;
   static volatile bool stats_thread_running = false;
   
   void* stats_update_thread_func(void* arg) {
       AppState* state = (AppState*)arg;
       
       while (stats_thread_running && state->training_in_progress) {
           update_sphere_stats(state, g_threaded_system);
           usleep(100000);  // 100ms
       }
       
       return NULL;
   }
   
   // Start in training_thread_func() before epoch loop
   stats_thread_running = true;
   pthread_create(&stats_update_thread, NULL, stats_update_thread_func, state);
   
   // Stop after training completes
   stats_thread_running = false;
   pthread_join(stats_update_thread, NULL);
   ```

4. **Same for Crawler**
   - Add similar thread in continuous_training.c
   - Use update_crawler_sphere_stats()

## Benefits
- Real-time visualization updates
- Spheres light up and pulse during training
- Users see actual training progress
- No blocking or performance impact

## Files to Modify
1. `app/training_thread.c` - Add stats update thread
2. `src/crawler/continuous_training.c` - Add stats update thread

## Testing
- Start training
- Watch spheres light up in real-time
- Verify stats update every 100ms
- Check thread cleanup on stop