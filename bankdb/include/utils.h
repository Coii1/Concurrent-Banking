/* Shared parsing and conversion helpers for account and trace inputs. */
#ifndef UTILS_H
#define UTILS_H

#include "bank.h"
#include "transaction.h"

#define MAX_TRANSACTIONS 256

int load_accounts_file(Bank* bank, const char* accounts_path);
int load_transactions_file(const char* trace_path, Transaction* txs[], int max_txs);
const char* op_type_to_string(OpType type);

#endif // UTILS_H
