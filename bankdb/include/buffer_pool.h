/* Declarations for bounded buffer pool structures, synchronization, and APIs. */
#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#define BUFFER_POOL_SIZE 5

#include <semaphore.h>
#include "bank.h"

typedef struct {
    int account_id;
    Account* data;
    bool in_use;
} BufferSlot;

typedef struct {
    BufferSlot slots[BUFFER_POOL_SIZE];
    sem_t empty_slots;
    sem_t full_slots;
    pthread_mutex_t pool_lock;
} BufferPool;

void init_buffer_pool(BufferPool* pool);

// Load account into buffer pool (producer)
void load_account(BufferPool* pool, int account_id);

// Unload account from buffer pool (consumer)
void unload_account(BufferPool* pool, int account_id);

#endif // BUFFER_POOL_H