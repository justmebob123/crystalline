#ifndef CRAWLER_H
#define CRAWLER_H

#include <pthread.h>

// Crawler state
typedef struct CrawlerState CrawlerState;

CrawlerState* crawler_init(const char* data_dir, const char* start_url, int max_pages);
void crawler_cleanup(CrawlerState* state);
void* crawler_thread_func(void* arg);

// Preprocessor state
typedef struct PreprocessorState PreprocessorState;

PreprocessorState* preprocessor_init(const char* data_dir);
void preprocessor_cleanup(PreprocessorState* state);
void* preprocessor_thread_func(void* arg);

// Tokenizer state
typedef struct TokenizerState TokenizerState;

TokenizerState* tokenizer_init(const char* data_dir);
void tokenizer_cleanup(TokenizerState* state);
void* tokenizer_thread_func(void* arg);

// Continuous training state
typedef struct ContinuousTrainingState ContinuousTrainingState;

ContinuousTrainingState* continuous_training_init(const char* data_dir, const char* model_path, 
                                                   void* model, int num_threads);
int continuous_training_start(ContinuousTrainingState* state, pthread_t* threads);
void continuous_training_stop(ContinuousTrainingState* state, pthread_t* threads);
void continuous_training_cleanup(ContinuousTrainingState* state);

#endif // CRAWLER_H
</file_path>