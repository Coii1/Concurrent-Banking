/* Transaction worker execution logic, scheduling, and operation dispatch. */
#include "transaction.h"
#include "timer.h"
#include <stdio.h>

static void print_operation(Operation* op) {
    switch (op->type) {
        case OP_DEPOSIT:
            printf("DEPOSIT account=%d amount=%d\n", op->account_id, op->amount_centavos);
            break;
        case OP_WITHDRAW:
            printf("WITHDRAW account=%d amount=%d\n", op->account_id, op->amount_centavos);
            break;
        case OP_TRANSFER:
            printf("TRANSFER from=%d to=%d amount=%d\n", op->account_id, op->target_account, op->amount_centavos);
            break;
        case OP_BALANCE:
            printf("BALANCE account=%d\n", op->account_id);
            break;
        default:
            printf("UNKNOWN OPERATION\n");
    }
}

void* execute_transaction(void* arg) {

    Transaction* tx = (Transaction*)arg;

    wait_until_tick(tx->start_tick);

    for (int i = 0; i < tx->num_ops; i++) {
        int target_tick = tx->ops[i].tick;
        wait_until_tick(target_tick);

        // Placeholder for now: just print progress
        printf("T%d reached op %d at tick %d\n", tx->tx_id, i, target_tick);
        print_operation(&tx->ops[i]);

    }

    return NULL;
}