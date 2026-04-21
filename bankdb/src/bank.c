/* Core account operations such as reads, writes, transfers, and balance updates. */
#include "bank.h"
#include <stdlib.h>
#include <stdio.h>

Bank* create_bank(){
    Bank *bank = malloc(sizeof(Bank));

    if (!bank) {
        fprintf(stderr, "Failed to allocate memory for bank\n");
        exit(EXIT_FAILURE);
    }

    bank->num_accounts = 0;
    pthread_mutex_init(&bank->bank_lock, NULL);
    return bank;
}

void destroy_bank(Bank* bank){
    pthread_mutex_destroy(&bank->bank_lock);
    //free accounts
    for (int i = 0; i < bank->num_accounts; i++) {
        pthread_mutex_destroy(&bank->accounts[i].lock);
        // free(&bank->accounts[i]); //accounts are stored by value in the bank struct, so we don't need to free them individually
    }
    free(bank);
}

Account* create_account(int account_id, int balance_centavos){
    Account *account = malloc(sizeof(Account));

    if (!account) {
        fprintf(stderr, "Failed to allocate memory for account\n");
        exit(EXIT_FAILURE);
    }
    
    account->account_id = account_id;
    account->balance_centavos = balance_centavos;
    pthread_mutex_init(&account->lock, NULL);
    return account;
    
}