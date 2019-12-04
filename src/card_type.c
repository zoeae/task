/*
 * card_type.c
 *
 */

#include <assert.h>
#include <string.h>
#include "card_type.h"


/* this is the card types "table"
 * it's implemented as a simple array
 * in this implementation, it's a read only object, that's
 * initialized to a given list values.
 * These values are hardcoded.
 * A better implementation would allow reading these values
 * from a file, like a config file
 *
 * lookups to this array are sequential, always like "full table scan"
 * stepping through the whole table looking for data,
 * A better implementation would use a hash map to enable fast access
 * although for very low cardinality data like card types it won't
 * make a difference.
 * Or it might be hash map that acts a a cache on the real database table
 * Also memcached can be evaluated as an im memory key value store,
 * although memcached is "out of process" architecture and will need
 * network traffic for access
 */
static Card_Type Cards[] = {
        { 1, "Visa" },
        { 2, "MasterCard" },
        { 3, "EFTPOS" },
        { 4, "Amex" },
        { 5, "JBC" },
        { 0, NULL }
};


/* checks if card type is valid */
bool card_type_is_valid(const char *name) {
  assert(name != NULL);
  return card_type_find_by_name(name);
}

/* find a card type in the table using it's name */
Card_Type *card_type_find_by_name(const char *name) {
  int i;

  assert(name != NULL);
  for (i = 0; Cards[i].name != NULL; i++) {
    if (strcmp(Cards[i].name, name) == 0) {
      return &Cards[i];
    }
  }
  return NULL;
}

/* find a card type by it's id */
Card_Type *card_type_find_by_id(card_type_id id) {
  int i;

  for (i = 0; Cards[i].id != 0; i++) {
    if (Cards[i].id == id) {
      return &Cards[i];
    }
  }
  return NULL;
}


/* vim: set et sm ai ts=2: */
