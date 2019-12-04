/*
 * terminal.h
 *
 */

#ifndef __TERMINAL_H
#define __TERMINAL_H

#include <stdbool.h>
#include <stdint.h>

#include "card_type.h"
#include "transaction_type.h"

#define N_CARDS 10
#define N_TRXS  5
#define N_TERMINALS  1000

/* have a specific, separate type for ids */
typedef uint32_t terminal_id;

/* this is the basic structure for terminal data
 * it contains
 * an ID, that should not be 0
 * and an array of card types ids that references card types
 *      a 0 in the array marks the end of data, this enables an early stop
 *      in scanning the array for references
 * and an array of transaction types ids that references transaction types
 *      a 0 in the array marks the end of data, this enables an early stop
 *      in scanning the array for references
 *
 * having an array with a fixed number of slots is:
 * innefficient in space, as you reserve more memory than needed. not used
 * positions still use memory
 * imposes an artificial limit.  when the array is full, no more data can
 * be inserted into it
 *
 * a better implementation would resize the arrays as needed to accomodate
 * new data, using malloc/free/realloc. 
 */
typedef struct terminal_data {
  terminal_id id;
  card_type_id cards[N_CARDS];
  transaction_type_id trxs[N_TRXS];
} Terminal_Data;


/* prototypes */
extern void terminal_init_data(Terminal_Data *t);
extern Terminal_Data *terminal_find_by_id(terminal_id id);
extern bool terminal_is_valid(Terminal_Data *t);
extern bool terminal_add(Terminal_Data *t);
extern char *terminal_to_json(Terminal_Data *t);
extern char *terminal_all_to_json(void);
extern bool terminal_load_json(Terminal_Data *t, const char *input);
extern bool terminal_add_card_type(Terminal_Data *t, const char *name);
extern bool terminal_add_transaction_type(Terminal_Data *t, const char *name);



#endif

/* vim: set et sm ai ts=2: */
