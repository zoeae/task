/*
 * card_type.h
 *
 */

#ifndef __CARD_TYPE_H
#define __CARD_TYPE_H

#include <stdbool.h>
#include <stdint.h>

/* have a specific, separate type for ids */
typedef uint32_t card_type_id;

/* this is the basic structure for card types
 * card types have an id, that's use for references from the terminal structure
 * and a name, that's a string representation for the id
 */
typedef struct card_type {
  card_type_id id;
  char *name;
} Card_Type;


/* prototypes */
extern bool card_type_is_valid(const char *name);
extern Card_Type *card_type_find_by_name(const char *name);
extern Card_Type *card_type_find_by_id(card_type_id id);

#endif

/* vim: set et sm ai ts=2: */
