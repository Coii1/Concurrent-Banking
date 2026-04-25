/* Declarations for timer thread control and logical clock helper functions. */

#ifndef TIMER_H
#define TIMER_H

#include <pthread.h>

// Global simulation clock (shared by all threads)
extern volatile int global_tick;
extern pthread_mutex_t tick_lock;
extern pthread_cond_t tick_changed;

// Timer thread increments clock every TICK_INTERVAL_MS
void* timer_thread(void* arg) ;

// Transactions wait until their start_tick
void wait_until_tick(int target_tick) ;

void timer_init(void);
void timer_destroy(void);
#endif // TIMER_H