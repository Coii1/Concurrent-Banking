/* Shared utilities for parsing input, validation, and error handling. */

#include "bank.h"
#include <stdio.h>
#include <stdlib.h>

static int init_account_slot(Account* slot, int account_id, int balance_centavos, int line_no, const char* line) {
    slot->account_id = account_id;
    slot->balance_centavos = balance_centavos;

    if (pthread_mutex_init(&slot->lock, NULL) != 0) {
        fprintf(stderr, "Failed to init account lock at line %d: %s", line_no, line);
        return -1;
    }

    return 0;
}

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
        
        
        if (bank->num_accounts >= MAX_ACCOUNTS) {
            fprintf(stderr, "Exceeded maximum number of accounts at line %d: %s", line_no, line);
            continue;
        }
        
        Account *slot = &bank->accounts[bank->num_accounts];
        if (init_account_slot(slot, account_id, balance_centavos, line_no, line) != 0) {
            continue;
        }

        bank->num_accounts++;
        /*
        No more malloc for temporary Account, so no leak.
        */
    }

    fclose(fp);


    return 0;
}