/*
 * transaction_type.c
 *
 */

#include <assert.h>
#include <string.h>
#include "transaction_type.h"


/* this is the transactions types "table"
 * it's implemented as a simple array
 * in this implementation, it's a read only object, that's initialized
 * to a given list of values
 * These values are hardcoded
 * A better implementation would allow reading these values from a file
 * like a config file
 *
 * lookups to this array are sequential, always like "full table scan"
 * stepping through the whole table lookwing for data
 * A better implementation would use a hash map to enable fast access
 * although for very low cardinality data like transaction types it won't make 
 * a difference
 */
static Transaction_Type Transactions[] = {
        { 91, "Cheque" },
        { 92, "Savings" },
        { 93, "Credit" },
        { 94, "Other" },
        { 0, NULL }
};

bool transaction_type_is_valid(const char *name) {
  assert(name != NULL);
  return transaction_type_find_by_name(name);
}

Transaction_Type *transaction_type_find_by_name(const char *name) {
  int i;

  assert(name != NULL);
  for (i = 0; Transactions[i].name != NULL; i++) {
    if (strcmp(Transactions[i].name, name) == 0) {
      return &Transactions[i];
    }
  }
  return NULL;
}

Transaction_Type *transaction_type_find_by_id(transaction_type_id id) {
  int i;

  for (i = 0; Transactions[i].id != 0; i++) {
    if (Transactions[i].id == id) {
      return &Transactions[i];
    }
  }
  return NULL;
}


/* vim: set et sm ai ts=2: */
