/* Shared utilities for parsing input, validation, and error handling. */

#include "bank.h"
#include "transaction.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            break; //stop processing if we reach max accounts, to avoid overflow;
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

int load_transactions_file(const char* trace_path, Transaction* txs[]) {
    FILE* trace_fp;
    char line[256];
    int line_no = 0;
    int tx_count = 0;
    int i;

    if (trace_path == NULL || txs == NULL || MAX_TRANSACTIONS <= 0) {
        return -1;
    }

    for (i = 0; i < MAX_TRANSACTIONS; i++) {
        txs[i] = NULL;
    }

    trace_fp = fopen(trace_path, "r");
    if (trace_fp == NULL) {
        return -1;
    }

	while (fgets(line, sizeof(line), trace_fp)) {
		char tx_label[32];
		int tx_id;
		char op_str[32];
		int tick;
		int account_id;
		int value1 = 0;
		int value2 = 0;
		int fields;
		int tx_index = -1;
		Transaction* tx;
		Operation* op;

		line_no++;

		if (line[0] == '#' || line[0] == '\n') continue;

		fields = sscanf(line, "%31s %d %31s %d %d %d", tx_label, &tick, op_str, &account_id, &value1, &value2);
		if (fields < 4) {
			fprintf(stderr, "Invalid trace format at line %d: %s", line_no, line);
			fclose(trace_fp);
			return -1;
		}

		tx_id = atoi((tx_label[0] == 'T' || tx_label[0] == 't') ? tx_label + 1 : tx_label);

		// Check if transaction already exists so we can append operations to it, otherwise create new transaction
		for (int i = 0; i < tx_count; i++) {
			if (txs[i] != NULL && txs[i]->tx_id == tx_id) {
				tx_index = i;					//tx_index refers to the index in the txs array where the transaction with tx_id is stored.
				break;
			}
		}
		/*
		 each line does an O(tx_count) search ⇒ total can become O(lines × transactions).
		 BUT no Works even if tx IDs are not contiguous (e.g., T1, T7, T500) as long as count ≤ MAX.
		 TODO: if this becomes a performance bottleneck, optimize by using a hash map or direct indexing 
		*/

		// create new transaction if not found, and add to txs array 
		if (tx_index < 0) {
			if (tx_count >= MAX_TRANSACTIONS) {
				fprintf(stderr, "Exceeded maximum number of transactions at line %d: %s", line_no, line);
				fclose(trace_fp);
				return -1;
			}

			txs[tx_count] = (Transaction*)calloc(1, sizeof(Transaction));   
			/*
			use calloc to zero-initialize the transaction struct, so we don't have to manually set num_ops and status to 0 and TX_RUNNING respectively.
			also avoids potential uninitialized memory issues.
			remember to free this later in main after we're done with the transactions.
			*/

			if (txs[tx_count] == NULL) {
				fclose(trace_fp);
				return -1;
			}

			txs[tx_count]->tx_id = tx_id;
			txs[tx_count]->start_tick = tick;
			txs[tx_count]->status = TX_RUNNING;
			tx_index = tx_count;
			tx_count++;
		}

		//Set new transaction's start_tick to earliest tick among its operations
		tx = txs[tx_index];
		if (tick < tx->start_tick) {
			tx->start_tick = tick;
		}

		if (tx->num_ops >= (int)(sizeof(tx->ops) / sizeof(tx->ops[0]))) {
			fprintf(stderr, "Too many operations for T%d at line %d\n", tx_id, line_no);
			fclose(trace_fp);
			return -1;
		}

		op = &tx->ops[tx->num_ops];
		memset(op, 0, sizeof(*op)); // clear's Operation struct to all-zero before filling fields to avoid leftover garbage values.
		op->tick = tick;

		if (strcmp(op_str, "DEPOSIT") == 0) {
			if (fields != 5) {
				fprintf(stderr, "DEPOSIT expects 5 fields at line %d: %s", line_no, line);
				fclose(trace_fp);
				return -1;
			}
			op->type = OP_DEPOSIT;
			op->account_id = account_id;
			op->amount_centavos = value1;
		} else if (strcmp(op_str, "WITHDRAW") == 0) {
			if (fields != 5) {
				fprintf(stderr, "WITHDRAW expects 5 fields at line %d: %s", line_no, line);
				fclose(trace_fp);
				return -1;
			}
			op->type = OP_WITHDRAW;
			op->account_id = account_id;
			op->amount_centavos = value1;
		} else if (strcmp(op_str, "BALANCE") == 0) {
			if (fields != 4) {
				fprintf(stderr, "BALANCE expects 4 fields at line %d: %s", line_no, line);
				fclose(trace_fp);
				return -1;
			}
			op->type = OP_BALANCE;
			op->account_id = account_id;
		} else if (strcmp(op_str, "TRANSFER") == 0) {
			if (fields != 6) {
				fprintf(stderr, "TRANSFER expects 6 fields at line %d: %s", line_no, line);
				fclose(trace_fp);
				return -1;
			}
			op->type = OP_TRANSFER;
			op->account_id = account_id;
			op->target_account = value1;
			op->amount_centavos = value2;
		} else {
			fprintf(stderr, "Unknown operation at line %d: %s", line_no, line);
			fclose(trace_fp);
			return -1;
		}

		tx->num_ops++;
	}

	fclose(trace_fp);
	return tx_count;
}