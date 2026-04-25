/* Transaction worker execution logic, scheduling, and operation dispatch. */
#include "transaction.h"
#include "timer.h"
#include <stdio.h>

void* execute_transaction(void* arg) {

    Transaction* tx = (Transaction*)arg;

    wait_until_tick(tx->start_tick);

    for (int i = 0; i < tx->num_ops; i++) {
        int target_tick = tx->ops[i].tick;
        wait_until_tick(target_tick);

        // Placeholder for now: just print progress
        printf("T%d reached op %d at tick %d\n", tx->tx_id, i, target_tick);
    }

    return NULL;
}