/*
 * transaction_type.h
 *
 */

#ifndef __TRANSACTION_TYPE_H
#define __TRANSACTION_TYPE_H

#include <stdbool.h>
#include <stdint.h>

/* have a specific, separate type for ids */
typedef uint32_t transaction_type_id;

/* this is the basic structure for transaction types
 * transaction types have an id, that's used for references from the terminal
 * structure
 * and a name, that's a string representation for the id
 */
typedef struct transaction_type {
  transaction_type_id id;
  char *name;
} Transaction_Type;

/* prototypes */
extern bool transaction_type_is_valid(const char *name);
extern Transaction_Type *transaction_type_find_by_name(const char *name);
extern Transaction_Type *transaction_type_find_by_id(transaction_type_id id);



#endif

/* vim: set et sm ai ts=2: */
