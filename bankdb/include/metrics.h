/* Declarations for runtime statistics collection and reporting interfaces. */
#ifndef METRICS_H
#define METRICS_H

#include <pthread.h>

extern long total_deposited;
extern long total_withdrawn;
extern pthread_mutex_t metrics_lock;

void metrics_init(void);
void record_deposit(int amount);
void record_withdraw(int amount);
void print_final_report(Bank* bank, Transaction* txs[], int txs_count, long initial_sum);

#endif