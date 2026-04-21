/* Shared utilities for parsing input, validation, and error handling. */

#include "bank.h"
#include <stdio.h>
#include <stdlib.h>

int load_accounts_file(Bank* bank, const char* accounts_path) {
    FILE* fp;
    char line[256];
	int line_no = 0;

	if (bank == NULL || accounts_path == NULL) {
		return -1;
	}

    fp = fopen(accounts_path, "r");
	if (fp == NULL) {
		return -1;
	}

    while (fgets(line, sizeof(line), fp)) {
        int account_id;
		int balance_centavos;
		int parsed_fields;
        line_no++;

        if (line[0] == '#' || line[0] == '\n') continue;

        parsed_fields = sscanf(line, "%d %d", &account_id, &balance_centavos);
        if (parsed_fields != 2) {
            fprintf(stderr, "Invalid format in accounts file at line %d: %s", line_no, line);
            continue; //skip lang yung invalid line, but keep processing the rest of the file
        }
        
        
        Account *account = create_account(account_id, balance_centavos);
        if (account == NULL) {
            fprintf(stderr, "Failed to create account for line %d: %s", line_no, line);
            continue;
        }

        if (bank->num_accounts >= MAX_ACCOUNTS) {
            fprintf(stderr, "Exceeded maximum number of accounts at line %d: %s", line_no, line);
            free(account);
            continue;
        }

        bank->accounts[bank->num_accounts++] = *account;
        /*
        Unsure: currently, we allocate and create an account struct with create_account, 
        then copy it into the bank's accounts array, 


        should we instead directly insert the created account into the bank's accounts array without copying?
        */
    }

    fclose(fp);


    return 0;
}