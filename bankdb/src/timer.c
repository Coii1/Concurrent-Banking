/* Timer thread implementation, clock ticks, and timeout/event signaling. */
#include "timer.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

// Global simulation clock (shared by all threads)
volatile int global_tick;
pthread_mutex_t tick_lock;
pthread_cond_t tick_changed;

static bool all_transactions_done = false; 

// Timer thread increments clock every TICK_INTERVAL_MS
void* timer_thread(void* arg) {
    // while (!all_transactions_done) {
    while (global_tick < 10) { //for testing, run the timer for 100 ticks then stop. Change back to while loop with all_transactions_done flag for final version.
        
        // usleep(*(int*)arg * 1000); //tick_interval_ms = 100ms for now, can make it configurable later if needed
        sleep(*(int*)arg); //for testing, tick every 1 second to make it easier to observe the transactions executing at their respective ticks. Change back to usleep with arg for final version.
        pthread_mutex_lock(&tick_lock);
        global_tick++;
        printf("Tick: %d\n", global_tick); //for debugging, can remove later
        
        // Wake all transactions waiting for this tick
        pthread_cond_broadcast(&tick_changed);
        
        pthread_mutex_unlock(&tick_lock);
    }
    return NULL;
}

// Transactions wait until their start_tick
void wait_until_tick(int target_tick) {
    pthread_mutex_lock(&tick_lock);
    while (global_tick < target_tick) {
        pthread_cond_wait(&tick_changed, &tick_lock);
    }
    pthread_mutex_unlock(&tick_lock);
}

void timer_init(void) {
    global_tick = 0;
    pthread_mutex_init(&tick_lock, NULL);
    pthread_cond_init(&tick_changed, NULL);
}

void timer_destroy(void) {
    pthread_mutex_destroy(&tick_lock);
    pthread_cond_destroy(&tick_changed);
}

