/* Declarations for lock manager APIs, lock ordering, and deadlock handling. */
#ifndef LOCK_MGR_H
#define LOCK_MGR_H

#include "bank.h"

// Function declarations for lock management
void lock_account(Account* account);
void unlock_account(Account* account);

#endif // LOCK_MGR_H