# Design Notes

## Overview
This project implements a concurrent in-memory banking system using pthreads. Each transaction runs in its own thread, operations are scheduled by a global tick timer, and account access is synchronized with per-account reader-writer locks. A bounded buffer pool models account loading with semaphores.

## Build and Run
From the bankdb/ directory:

    make clean
    make
    ./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_simple.txt --tick-ms=100

Manual build (without Makefile):

    gcc -Wall -Wextra -std=c11 -Iinclude src/*.c -o bin/bankdb -pthread
    ./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_simple.txt --tick-ms=100

## Concurrency Model
- Timer thread advances a global tick and wakes waiting transactions.
- Each transaction thread waits for its start tick, then waits for each operation tick.
- Account access uses per-account pthread_rwlock_t to allow concurrent reads and exclusive writes.

## Deadlock Handling
- Deadlock prevention is enforced by lock ordering in transfers.
- Locks are acquired by ascending account ID to avoid circular wait.
- The CLI only supports prevention (no detection mode).

## Buffer Pool
- A fixed-size buffer pool (BUFFER_POOL_SIZE) simulates loading accounts.
- load_account() waits on empty_slots, marks a slot in use, then signals full_slots.
- unload_account() waits on full_slots, frees the slot, then signals empty_slots.
- Transactions call load_account/unload_account around each account access.

## Metrics and Reporting
- metrics_init() creates the mutex used by metrics counters before any threads start.
- Deposits and withdrawals update total counters under a mutex.
- Final report prints the actual transaction status (COMMITTED or ABORTED).

## Data Structures
- Bank contains a fixed array of Account records (MAX_ACCOUNTS) and a bank lock.
- Account includes id, balance, and a pthread_rwlock_t.
- Transactions store operations with per-op ticks and a start tick.

## Known Limitations
- Account lookup is linear by account_id, which is acceptable for lab scale but not optimal for large datasets.
- The bank instance and buffer pool are global for simplicity.

## Test Commands
    ./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_simple.txt --tick-ms=100
    ./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_readers.txt --tick-ms=100
    ./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_abort.txt --tick-ms=100
    ./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_deadlock.txt --tick-ms=100
    ./bin/bankdb --accounts=tests/accounts.txt --trace=tests/trace_buffer.txt --tick-ms=100
