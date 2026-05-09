/* Bounded buffer implementation using synchronization primitives. */
#include "buffer_pool.h"
#include <stdio.h>

void init_buffer_pool(BufferPool* pool) {
    // Make all spots on the table as empty
    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {
        pool->slots[i].in_use = false;
        pool->slots[i].account_id = -1;
        pool->slots[i].data = NULL;
    }
    // Fill empty_slots bowl with tickets
    sem_init(&pool->empty_slots, 0, BUFFER_POOL_SIZE);
    // Empty full_slots bowl
    sem_init(&pool->full_slots, 0, 0);             
    // Ensure only one transaction worker can look at the table at a time to see which spots are empty or full
    pthread_mutex_init(&pool->pool_lock, NULL);     
}

// When a transaction worker needs an account, take a ticket from the empty_slots bowl
void load_account(BufferPool* pool, int account_id) {
    sem_wait(&pool->empty_slots); // Block if pool is full

    // Once there is space, grab pool lock so no one else can move folders around
    pthread_mutex_lock(&pool->pool_lock);
    // Find a free slot and "load" the account
    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {
        if (!pool->slots[i].in_use) {
            pool->slots[i].account_id = account_id;
            pool->slots[i].in_use = true;
            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);
    // Put ticket into the full_slots bowl to let everyone know there is a new acc to work on 
    sem_post(&pool->full_slots);
}


// Clean up to put account back to the filing cabinet to make room for others
void unload_account(BufferPool* pool, int account_id) {
    // Take a ticket from full_slots bowl
    sem_wait(&pool->full_slots);
    // Lock the table
    pthread_mutex_lock(&pool->pool_lock);

    // Clear that spot on the table as empty
    for (int i = 0; i < BUFFER_POOL_SIZE; i++) {
        if (pool->slots[i].in_use && pool->slots[i].account_id == account_id) {
            pool->slots[i].in_use = false;
            pool->slots[i].account_id = -1;
            break;
        }
    }

    pthread_mutex_unlock(&pool->pool_lock);
    // Put ticket back into empty_slots blow and let next transaction worker know there is a free spot available
    sem_post(&pool->empty_slots);
}