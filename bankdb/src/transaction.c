/* Transaction worker execution logic, scheduling, and operation dispatch. */
#include "bank.h" // Include bank.h to access the global bank instance and account operations. Remove when buffer pool is implemented.
#include "transaction.h"
#include "timer.h"
#include <stdio.h>
#include <stdbool.h>

static void print_loaded_accounts(Bank* bank) {
    printf("Loaded %d accounts:\n", bank->num_accounts);
    for (int i = 0; i < bank->num_accounts; i++) {
        printf("Account ID: %d, Balance: %d centavos\n", bank->accounts[i].account_id, bank->accounts[i].balance_centavos);
    }
}

static int get_balance(int account_id) {
    printf("Getting balance for account %d (placeholder value)\n", account_id);
    for(int i = 0; i < bank->num_accounts; i++) {
        if (bank->accounts[i].account_id == account_id) {
            return bank->accounts[i].balance_centavos;
        }
    }
    print_loaded_accounts(bank);
    return 0;
}

static void deposit(int account_id, int amount_centavos) {
    printf("Depositing %d centavos to account %d\n", amount_centavos, account_id);
    for(int i = 0; i < bank->num_accounts; i++) {
        if (bank->accounts[i].account_id == account_id) {
            bank->accounts[i].balance_centavos += amount_centavos;
            print_loaded_accounts(bank);
            break;
        }
    }
}

static bool withdraw(int account_id, int amount_centavos) {
    printf("Withdrawing %d centavos from account %d\n", amount_centavos, account_id);
    for(int i = 0; i < bank->num_accounts; i++) {
        if (bank->accounts[i].account_id == account_id) {
            if (bank->accounts[i].balance_centavos >= amount_centavos) {
                bank->accounts[i].balance_centavos -= amount_centavos;
                print_loaded_accounts(bank);
                return true;
            } else {
                printf("Insufficient funds in account %d\n", account_id);
                print_loaded_accounts(bank);
                return false;
            }
        }
    }
    
    return false; // Account not found, treat as insufficient funds for simplicity for now
}

static bool transfer(int from_id, int to_id, int amount_centavos) { //TODO: Implement proper deadlock-free locking for transfers. For now, just lock both accounts in a consistent order to avoid deadlocks, but this is not ideal for performance and can lead to contention. We will implement a more sophisticated locking strategy later.
    printf("Transferring %d centavos from account %d to account %d\n", amount_centavos, from_id, to_id);
    // int first = (from_id < to_id) ? from_id : to_id;
    // int second = (from_id < to_id) ? to_id : from_id;
    
    /*
    after reaching this point, it is evident that given millions of accounts in a bank, having to search for the index given the id is very inefficient.
    we should have a more efficient way to map account_id to its index in the accounts array, such as a hash map or a direct mapping if account ids are guaranteed to be dense and within a certain range.
    for now, we will just do a linear search to find the indices of the from and to accounts, but this is a clear area for optimization in the future.
     */
    
    int from_index = -1;
    int to_index = -1;
    for (int i = 0; i < bank->num_accounts; i++) {
        if (bank->accounts[i].account_id == from_id) {
            from_index = i;
        }
        if (bank->accounts[i].account_id == to_id) {
            to_index = i;
        }
        if (from_index >= 0 && to_index >= 0) {
            break;
        }
    }
    if (from_index < 0 || to_index < 0) {
        printf("One or both accounts not found for transfer: from_id=%d to_id=%d\n", from_id, to_id);
        print_loaded_accounts(bank);
        return false;
    }
    
    Account* acc_first = &bank->accounts[from_index];
    Account* acc_second = &bank->accounts[to_index];
    
    pthread_rwlock_wrlock(&acc_first->lock);
    pthread_rwlock_wrlock(&acc_second->lock);
    
    // Check sufficient funds
    Account* from_acc = &bank->accounts[from_index];
    if (from_acc->balance_centavos < amount_centavos) {
        pthread_rwlock_unlock(&acc_second->lock);
        pthread_rwlock_unlock(&acc_first->lock);
        printf("Insufficient funds in account %d for transfer\n", from_id);
        print_loaded_accounts(bank);
        return false;
    }
    
    // Perform transfer
    bank->accounts[from_index].balance_centavos -= amount_centavos;
    bank->accounts[to_index].balance_centavos += amount_centavos;
    
    pthread_rwlock_unlock(&acc_second->lock);
    pthread_rwlock_unlock(&acc_first->lock);
    print_loaded_accounts(bank);
    return true;
}

void* execute_transaction(void* arg) {

    Transaction* tx = (Transaction*)arg;

    wait_until_tick(tx->start_tick);

    for (int i = 0; i < tx->num_ops; i++) {
        Operation* op = &tx->ops[i];

        int target_tick = op->tick;
        wait_until_tick(target_tick);

        // // Placeholder for now: just print progress
        // printf("T%d reached op %d at tick %d\n", tx->tx_id, i, target_tick);
        // print_operation(op);

        switch (op->type) {
            case OP_DEPOSIT:
                deposit(op->account_id, op->amount_centavos);
                break;
                
            case OP_WITHDRAW:
                if (!withdraw(op->account_id, op->amount_centavos)) {
                    // Insufficient funds - abort transaction
                    tx->status = TX_ABORTED;
                    return NULL;
                }
                break;
                
            case OP_TRANSFER:
                if (!transfer(op->account_id, op->target_account,
                              op->amount_centavos)) {
                    tx->status = TX_ABORTED;
                    return NULL;
                }
                break;
                
            case OP_BALANCE:
                int balance = get_balance(op->account_id);
                printf("T%d: Account %d balance = PHP %d.%02d\n", 
                       tx->tx_id, op->account_id, 
                       balance / 100, balance % 100);
                break;
        }

    }

    return NULL;
}