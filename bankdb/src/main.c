/* Entry point: CLI parsing, configuration loading, and subsystem initialization. */

#include <stdio.h>
#include <stdlib.h>
#include "bank.h"
#include "lock_mgr.h"
#include "transaction.h"
#include "timer.h"
#include "utils.h"

static const char* op_name(OpType t) {
    switch (t) {
        case OP_DEPOSIT: return "DEPOSIT";
        case OP_WITHDRAW: return "WITHDRAW";
        case OP_TRANSFER: return "TRANSFER";
        case OP_BALANCE: return "BALANCE";
        default: return "UNKNOWN";
    }
}

// just for debugging, can remove later
static void print_loaded_transactions(Transaction* txs[], int max_txs) {
    int tx_printed = 0;

    for (int i = 0; i < max_txs; i++) {
        Transaction* tx = txs[i];
        if (tx == NULL) {
            break;
        }

        tx_printed++;
        printf("\nT%d  start_tick=%d  num_ops=%d\n",
            tx->tx_id, tx->start_tick, tx->num_ops);

        for (int j = 0; j < tx->num_ops; j++) {
            Operation* op = &tx->ops[j];

            if (op->type == OP_TRANSFER) {
                printf("  op[%d] tick=%d %s from=%d to=%d amount=%d\n",
                    j, op->tick, op_name(op->type),
                    op->account_id, op->target_account, op->amount_centavos);
            } else if (op->type == OP_BALANCE) {
                printf("  op[%d] tick=%d %s account=%d\n",
                    j, op->tick, op_name(op->type), op->account_id);
            } else {
                printf("  op[%d] tick=%d %s account=%d amount=%d\n",
                    j, op->tick, op_name(op->type),
                    op->account_id, op->amount_centavos);
            }
        }
    }

    printf("\nTotal loaded transactions: %d\n", tx_printed);
}

static void print_loaded_accounts(Bank* bank) {
    printf("Loaded %d accounts:\n", bank->num_accounts);
    for (int i = 0; i < bank->num_accounts; i++) {
        printf("Account ID: %d, Balance: %d centavos\n", bank->accounts[i].account_id, bank->accounts[i].balance_centavos);
    }
}

int main(int argc, char* argv[]) {

    //parse CLI args for accounts file and trace file paths.
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <accounts_file> <trace_file>\n", argv[0]);   
        exit(EXIT_FAILURE);
    }
    char *accounts_path = argv[1];
    char *trace_path = argv[2];
    

    //initialize bank, accounts, and locks.  
    Bank* bank = create_bank();
    
    //declare transactions array in main for now. NOT SURE if it should be added in the bank struct
    Transaction *txs[MAX_TRANSACTIONS];
    
    if (load_accounts_file(bank, accounts_path)){
        fprintf(stderr, "Failed to load accounts from %s\n", argv[1]);
        destroy_bank(bank);
        return 1;
    }
    // load transaction trace
    if (load_transactions_file(trace_path, txs) < 0) {
        fprintf(stderr, "Failed to load transactions from %s\n", trace_path);
        destroy_bank(bank);
        return 1;    
    }


    print_loaded_accounts(bank);
    

    print_loaded_transactions(txs, MAX_TRANSACTIONS);

    
    //initialize timer thread and logical clock
    // pthread_t timer_tid;
    // if (pthread_create(&timer_tid, NULL, timer_thread, NULL) != 0) {
    //     fprintf(stderr, "Failed to create timer thread\n");
    //     // free(txs);
    //     destroy_bank(bank);// TODO: maybe put this repetitive code to a cleanup function
    //     return 1;
    // }


    // TODO:  create transaction threads, and execute transactions

    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (txs[i] != NULL) {
            free(txs[i]);
        }
    }
    destroy_bank(bank);
    return 0; 
}

