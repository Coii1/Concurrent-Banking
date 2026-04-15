/* Declarations for bank and account data structures and related interfaces. */
#ifndef BANK_H
#define BANK_H
#define MAX_ACCOUNTS 100
#define _XOPEN_SOURCE 500  // or // #define _GNU_SOURCE for rwlocks

#include <stdbool.h>
#include <pthread.h>

typedef struct {
    int account_id;         
    int balance_centavos;   
    pthread_rwlock_t lock;   // Per-account locks // might say identifier "pthread_rwlock_t" is undefined, but it is defined in <pthread.h> when _XOPEN_SOURCE or _GNU_SOURCE is defined, this is just an IDE issue
} Account;

typedef struct {
    Account accounts[MAX_ACCOUNTS];
    int num_accounts;
    pthread_mutex_t bank_lock;  // Protects bank metadata
} Bank;



Bank* create_bank();

Account* create_account(Bank* bank, int account_number, double initial_balance);

void deposit(int account_id, int amount_centavos);

bool withdraw(int account_id, int amount_centavos);

bool transfer(int from_id, int to_id, int amount_centavos);

int get_balance(int account_id);

#endif 