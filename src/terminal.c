/*
 * terminal.c
 *
 */

#include <assert.h>
#include <string.h>
#include "terminal.h"
#include "jansson.h"

/* these constants are for encoding/decoding types in JSON */
#define TERMINAL_ID_JSON "id"
#define CARD_TYPE_JSON "CardType"
#define TRANSACTION_TYPE_JSON "TransactionType"

/* this is the terminals "table"
 * it's implemented as a simple array
 * in this implementation, there's always "full table scans" stepping
 * through the whole table looking for data.
 * this same "full table scan" is used by find_by_id and find_by_name functions
 * and in add (aka insert), when looking for an empty slot
 *
 * this terminals "table" should be a proper database table in a real world
 * implementation.  If kept in memory, this should not be an array but
 * a hash map to provide efficient lookups
 * There's a common architecture that should be explored, and it's using
 * memcached as an in memory (but out of process and hence requieres network
 * traffic) as a cache for terminal data
 *
 * Important:  note that methods that deal with terminals should be
 * reentrant (thread safe). This is because the terminals table is a shared
 * object used by different concurrent threads of libmicrohttp.
 * All operations that change content should be protected by a mutex.
 * In general, it looks like all writers should be sync so only one of
 * them can change the structure at a time (that is, add or delete terminals)
 * Readers shouldn't be blocked by other readers, but they should be blocked
 * by writers as the write operation can affect the lookups.
 * The current implementation IS NOT THREAD SAFE
 * 
 */
static Terminal_Data Terminals[N_TERMINALS];

/* copies terminal data */
static void terminal_copy(Terminal_Data *a, Terminal_Data *b) {
  assert(a != NULL);
  assert(b != NULL);
  memcpy(a, b, sizeof(Terminal_Data));
}

/* generates a new terminal id
 * this is a simple implementation that generates sequential ids
 */
static terminal_id new_terminal_id(void) {
  static terminal_id id_sequence = 1;

  return id_sequence++;
}

/* initializes the terminal data, setting everything to 0
 * this will clear any references to card types and transactions types
 */
void terminal_init_data(Terminal_Data *t) {
  int i;

  assert(t != NULL);
  t->id = 0;
  for (i = 0; i < N_CARDS; i++) {
    t->cards[i] = 0;
  }
  for (i = 0; i < N_TRXS; i++) {
    t->trxs[i] = 0;
  }
}

/* find a terminal in the table using it's id */
Terminal_Data *terminal_find_by_id(terminal_id id) {
  int i;

  if (id == 0) {
    return NULL;
  }

  for (i = 0; i < N_TERMINALS; i++) {
    if (Terminals[i].id == id) {
      return &Terminals[i];
    }
  }
  return NULL;
}

/* validate a terminal data information
 * for a terminal to be valid, it has to reference valid cards types
 * as well as valid transaction types
 */
bool terminal_is_valid(Terminal_Data *t) {
  int i;

  assert(t != NULL);

  for (i = 0; i < N_CARDS && t->cards[i] != 0; i++) {
    if (card_type_find_by_id(t->cards[i]) == NULL) {
      return false;
    }
  }

  for (i = 0; i < N_TRXS && t->trxs[i] != 0; i++) {
    if (transaction_type_find_by_id(t->trxs[i]) == NULL) {
      return false;
    }
  }

  return true;
}

/* add / insert a new terminal in the terminals table
 */
bool terminal_add(Terminal_Data *t) {
  int i;

  assert(t != NULL);
  /* terminal should be a new terminal */
  assert(t->id == 0);
  /* all terminal data should be valid */
  assert(terminal_is_valid(t));

  /* find an empty slot in the terminals table
   * this simply scans the whole table looking for an empty slot (id == 0)
   * a more efficient lookup could be implemented, but that would need a
   * different representation
   */
  for (i = 0; i < N_TERMINALS; i++) {
    if (Terminals[i].id == 0) {
      /* empty slot */

      /* generate a new terminal id for this terminal */
      t->id = new_terminal_id();

      /* copy terminal data to the terminal table */
      terminal_copy(&Terminals[i], t);
      return true;
    }
  }

  /* can not insert into terminal table.  the table is full */
  return false;
}

/* prepare for encoding to json
 * this is a helper function that gets a jansson
 * representation of the terminal data
*/
static json_t *terminal_prepare_json(Terminal_Data *t) {
  Card_Type *ct;
  Transaction_Type *tt;
  char *p;
  int i;
  json_t *json;

  assert(terminal_is_valid(t));

  /* initialize the json structure */
  json = json_object();

  /* add the terminal id */
  json_object_set(json, TERMINAL_ID_JSON, json_integer(t->id));

  /* add the card type array */
  json_t *cta = json_array();
  for (i = 0; i < N_CARDS && t->cards[i] != 0; i++) {
    ct = card_type_find_by_id(t->cards[i]);
    json_array_append(cta, json_string(ct->name));
  }
  json_object_set(json, CARD_TYPE_JSON, cta);

  /* add the transaction type array */
  json_t *tta = json_array();
  for (i = 0; i < N_TRXS && t->trxs[i] != 0; i++) {
    tt = transaction_type_find_by_id(t->trxs[i]);
    json_array_append(tta, json_string(tt->name));
  }
  json_object_set(json, TRANSACTION_TYPE_JSON, tta);

  return json;
}

/* encode as json all terminal data
* the returned pointer must be freed by the caller
*/
char *terminal_to_json(Terminal_Data *t) {
  json_t *json;
  char *p;

  assert(terminal_is_valid(t));

  json = terminal_prepare_json(t);

  /* generate the json encoded object as a string */
  p = json_dumps(json, JSON_INDENT(1));

  /* clear all memory allocated for json */
  json_object_clear(json);

  /* the returned pointer must be freed by the caller */
  return p;
}

char *terminal_all_to_json(void) {
  int i;
  char *p;
  json_t *json;

  /* initialize the json structure */
  json = json_array();

  for (i = 0; i < N_TERMINALS; i++) {
    if (Terminals[i].id != 0) {
      json_array_append(json, terminal_prepare_json(&Terminals[i]));
    }
  }

  /* generate the json encoded object as a string */
  p = json_dumps(json, JSON_INDENT(1));

  /* clear all memory allocated for json */
  json_object_clear(json);

  return p;
}


/* decode the received JSON and build the terminal data structure
 * perform validation
 */
bool terminal_load_json(Terminal_Data *t, const char *input) {
  int i;
  json_t *json;
  json_error_t json_err;
  int error_seen = false;

  assert(t != NULL);

  /* initialize the receiving structure for terminal data */
  terminal_init_data(t);

  /* initialize the json structure */
  json = json_loads(input, JSON_DECODE_ANY, &json_err);
  if (json == NULL) {
    /* TODO */
#if 0
    fprintf(stderr,
      "Error parsing JSON: [%s]\nline: [%d] column: [%d] position: [%d]\n",
      json_err.text,
      json_err.line,
      json_err.column,
      json_err.position
    );
#endif
    return false;
  }

  /* get the card types data */
  json_t *cta = json_object_get(json, CARD_TYPE_JSON);
  if (!json_is_array(cta)) {
#if 0
    /* TODO */
    fprintf(stderr,
      "Error parsing JSON: %s information is missing\n",
      CARD_TYPE_JSON
    );
#endif
    json_object_clear(json);
    return false;
  }
  /* check that every value received in the array
   * is a string, and that the string value is a valid one that
   * can be mapped to a type id
   */
  for (i = 0; i < json_array_size(cta); i++) {
    json_t *ctn = json_array_get(cta, i);
    if (json_is_string(ctn)) {
      if (! terminal_add_card_type(t, json_string_value(ctn))) {
        error_seen = true;
#if 0
        /* TODO */
        fprintf(stderr,
          "Error parsing JSON: %s value [%s] is invalid\n",
          CARD_TYPE_JSON,
          json_string_value(ctn)
        );
#endif
      }
    }
  }

  /* get the transaction types data */
  json_t *tta = json_object_get(json, TRANSACTION_TYPE_JSON);
  if (!json_is_array(tta)) {
    /* TODO */
#if 0
    fprintf(stderr,
      "Error parsing JSON: %s information is missing\n",
      "TransactonType"
    );
#endif
    json_object_clear(json);
    return false;
  }
  /* check that every value received in the array
   * is a string, and that the string value is a valid one that
   * can be mapped to a type id
   */
  for (i = 0; i < json_array_size(tta); i++) {
    json_t *ttn = json_array_get(tta, i);
    if (json_is_string(ttn)) {
      if (! terminal_add_transaction_type(t, json_string_value(ttn))) {
        error_seen = true;
        /* TODO */
#if 0
        fprintf(stderr,
          "Error parsing JSON: %s value [%s] is invalid\n",
          TRANSACTION_TYPE_JSON,
          json_string_value(ttn)
        );
#endif
      }
    }
  }

  /* clean all memory allocated for json */
  json_object_clear(json);

  /* at this point all terminal data should be valid */
  assert(terminal_is_valid(t));

  /* if any error was seen during JSON parsing, return an error
   * although some data could have been parsed succesfully
   */
  return !error_seen;
}

/* add a card type to this terminal */
bool terminal_add_card_type(Terminal_Data *t, const char *name) {
  int i;
  Card_Type *ct;

  assert(t != NULL);
  assert(name != NULL);

  if ((ct = card_type_find_by_name(name)) == NULL) {
    return false;
  }

  for (i = 0; i < N_CARDS && t->cards[i] != 0; i++) {
    if (t->cards[i] == ct->id) {
      /* value is already present in the array,
       * and should not be duplicated
       */
      return true;
    }
  }
  if (i == N_CARDS) {
    /* no space in structure to insert this data
     * inserting will corrupt memory
     */
    return false;
  }

  /* the card type can be inserted in the relationship */
  t->cards[i] = ct->id;
  return true;
}

/* add a transaction type to this terminal */
bool terminal_add_transaction_type(Terminal_Data *t, const char *name) {
  int i;
  Transaction_Type *tt;

  assert(t != NULL);
  assert(name != NULL);

  if ((tt = transaction_type_find_by_name(name)) == NULL) {
    return false;
  }

  for (i = 0; i < N_TRXS && t->trxs[i] != 0; i++) {
    if (t->trxs[i] == tt->id) {
      /* value is already present in the array,
       * and should not be duplicated
       */
      return true;
    }
  }
  if (i == N_TRXS) {
    /* no space in structure to insert this data
     * inserting will corrupt memory
     */
    return false;
  }

  /* the card type can be inserted in the relationship */
  t->trxs[i] = tt->id;
  return true;
}


/* vim: set et sm ai ts=2: */
