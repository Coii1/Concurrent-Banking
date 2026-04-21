/* Entry point: CLI parsing, configuration loading, and subsystem initialization. */

#include <stdio.h>
#include <stdlib.h>
#include "bank.h"
#include "lock_mgr.h"
#include "transaction.h"
#include "timer.h"
#include "utils.h"

Bank* create_bank(void);
void destroy_bank(Bank* bank);

int main(int argc, char* argv[]) {

    //parse CLI args for accounts file and trace file paths.
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <accounts_file> <trace_file>\n", argv[0]);   
        exit(EXIT_FAILURE);
    }
    char *accounts_path = argv[1];
    char *trace_path = argv[2];
    /*
    Note: current cli interface is 
    ./bin/bankdb tests/accounts.txt tests/trace_simple.txt

    instead of $ ./bankdb --accounts=accounts.txt --trace=trace.txt \
           --deadlock=prevention --tick-ms=100
    
    for simplcity and initial development.
    TODO: implement proper CLI parsing with --accounts= 
    */



    //initialize bank, accounts, and locks.
    
    Bank* bank = create_bank();
    
    if (load_accounts_file(bank, accounts_path)){
        fprintf(stderr, "Failed to load accounts from %s\n", argv[1]);
        destroy_bank(bank);
        return 1;
    }

   //print loaded accounts for verification
    printf("Loaded %d accounts:\n", bank->num_accounts);
    for (int i = 0; i < bank->num_accounts; i++) {
        printf("Account ID: %d, Balance: %d centavos\n", bank->accounts[i].account_id, bank->accounts[i].balance_centavos);
    }

    destroy_bank(bank);
    return 0;
}

/*
To run:
cd bankdb
gcc -Wall -Wextra -std=c11 -Iinclude src/*.c -o bin/bankdb -pthread
./bin/bankdb tests/accounts.txt tests/trace_simple.txt
*/