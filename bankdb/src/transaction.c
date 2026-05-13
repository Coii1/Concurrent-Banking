/* Transaction worker execution logic, scheduling, and operation dispatch when user wants to move mo. */
#include "bank.h" // Include bank.h to access the global bank instance and account operations. Remove when buffer pool is implemented.
#include "transaction.h"
#include "timer.h"
#include "metrics.h"
#include "buffer_pool.h"
#include <stdio.h>
#include <stdbool.h>

extern BufferPool buffer_pool;

// Helper to find account with matching id and lock it appropriately
static Account* find_account(int account_id) {
    for(int i = 0; i < bank->num_accounts; i++) {
        if (bank->accounts[i].account_id == account_id) {
            return &bank->accounts[i];
        }
    }
    return NULL;
}

// static void print_loaded_accounts(Bank* bank) {
//     printf("Loaded %d accounts:\n", bank->num_accounts);
//     for (int i = 0; i < bank->num_accounts; i++) {
//         printf("Account ID: %d, Balance: %d centavos\n", bank->accounts[i].account_id, bank->accounts[i].balance_centavos);
//     }
// }

// Handle logic race between get_balance and transfer
// Sol: Wrap every account access in a read/write lock

// rdlock before reading balance and unlock after - multiple workers can check the account balance at the same time without changing anything
static int get_balance(int account_id) {
    printf("Getting balance for account %d (placeholder value)\n", account_id);
    load_account(&buffer_pool, account_id);
    Account* acc = find_account(account_id);
    if (!acc) {
        unload_account(&buffer_pool, account_id);
        return 0;
    }

    // Add Read Lock for the balance inquiry
    pthread_rwlock_rdlock(&acc->lock);
    int balance = acc->balance_centavos;
    pthread_rwlock_unlock(&acc->lock);
    unload_account(&buffer_pool, account_id);

    return balance;
}

// rwlock so no one else can look at the account until done with the update (deposit and withdraw)
static void deposit(int account_id, int amount_centavos) {
    printf("Depositing %d centavos to account %d\n", amount_centavos, account_id);
    load_account(&buffer_pool, account_id);
    Account* acc = find_account(account_id);
    if (!acc) {
        unload_account(&buffer_pool, account_id);
        return;
    }

    // Add Write Lock for modification
    pthread_rwlock_wrlock(&acc->lock);
    acc->balance_centavos += amount_centavos;
    pthread_rwlock_unlock(&acc->lock);

    unload_account(&buffer_pool, account_id);

    record_deposit(amount_centavos);
}

static bool withdraw(int account_id, int amount_centavos) {
    printf("Withdrawing %d centavos from account %d\n", amount_centavos, account_id);
    load_account(&buffer_pool, account_id);
    Account* acc = find_account(account_id);
    if (!acc) {
        unload_account(&buffer_pool, account_id);
        return false;
    }

    // Add Write Lock for modification
    pthread_rwlock_wrlock(&acc->lock);
    if (acc->balance_centavos >= amount_centavos) {
        acc->balance_centavos -= amount_centavos;
        pthread_rwlock_unlock(&acc->lock);

        unload_account(&buffer_pool, account_id);

        record_withdraw(amount_centavos);
        return true;
    } 
    
    pthread_rwlock_unlock(&acc->lock);
    unload_account(&buffer_pool, account_id);
    printf("Insufficient funds in account %d\n", account_id);
    return false; // Account not found, treat as insufficient funds for simplicity for now
}

// use rwlock wrlock for both source and destination accounts before changing balances
static bool transfer(int from_id, int to_id, int amount_centavos) {
    int from_index = -1;
    int to_index = -1;
    int i;
    bool success = false;
    Account *acc_first, *acc_second;
    int first_id;
    int second_id;

    if (from_id == to_id || amount_centavos <= 0) return false;

    printf("Transferring %d centavos from account %d to account %d\n", amount_centavos, from_id, to_id);

    // Locate account indices
    for (i = 0; i < bank->num_accounts; i++) {
        if (bank->accounts[i].account_id == from_id) from_index = i;
        if (bank->accounts[i].account_id == to_id) to_index = i;
    }

    if (from_index < 0 || to_index < 0) return false;

    // Deadlock Prevention: Always lock the lower ID first
    if (from_id < to_id) {
        acc_first = &bank->accounts[from_index];
        acc_second = &bank->accounts[to_index];
        first_id = from_id;
        second_id = to_id;
    } else {
        acc_first = &bank->accounts[to_index];
        acc_second = &bank->accounts[from_index];
        first_id = to_id;
        second_id = from_id;
    }

    load_account(&buffer_pool, first_id);
    load_account(&buffer_pool, second_id);

    pthread_rwlock_wrlock(&acc_first->lock);
    pthread_rwlock_wrlock(&acc_second->lock);

    if (bank->accounts[from_index].balance_centavos >= amount_centavos) {
        bank->accounts[from_index].balance_centavos -= amount_centavos;
        bank->accounts[to_index].balance_centavos += amount_centavos;
        success = true;
    }

    pthread_rwlock_unlock(&acc_second->lock);
    pthread_rwlock_unlock(&acc_first->lock);

    unload_account(&buffer_pool, second_id);
    unload_account(&buffer_pool, first_id);

    if (success) {
        record_withdraw(amount_centavos);
        record_deposit(amount_centavos);
    }

    return success;
}

void* execute_transaction(void* arg) {

    Transaction* tx = (Transaction*)arg;

    // Wait for scheduled start by the timer
    wait_until_tick(tx->start_tick);

    // Capture metrics
    pthread_mutex_lock(&tick_lock);
    tx->actual_start = global_tick;
    pthread_mutex_unlock(&tick_lock);

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
                    break;
                }
                break;
                
            case OP_TRANSFER:
                if (!transfer(op->account_id, op->target_account,
                              op->amount_centavos)) {
                    tx->status = TX_ABORTED;
                    break;
                }
                break;
                
            case OP_BALANCE:
                int balance = get_balance(op->account_id);
                printf("T%d: Account %d balance = PHP %d.%02d\n", 
                       tx->tx_id, op->account_id, 
                       balance / 100, balance % 100);
                break;
        }

        if (tx->status == TX_ABORTED) {
            break;
        }

    }

    pthread_mutex_lock(&tick_lock);
    tx->actual_end = global_tick;
    pthread_mutex_unlock(&tick_lock);

    if (tx->status != TX_ABORTED) {
        tx->status = TX_COMMITTED;
    }
    return NULL;
}