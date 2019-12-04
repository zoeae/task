/*
 * dispatcher.h
 *
 */

#ifndef __DISPATCHER_H
#define __DISPATCHER_H

#include <stdbool.h>
#include <stdint.h>
#include "microhttpd.h"

/* this is dispatch structure
 * for every URL base, we have 5 function pointers, one for every HTTP verb
 * one for GET to handle READ actions
 * one for POST to handle CREATE actions
 * one for PUT to handle UPDATE/CREATE actions
 * one for PATCH to handle UPDATE actions
 * one for DELETE to handle DELETE actions
 */
typedef struct  {
  char *url;
  int (*dispatch_function[5])(
        struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *upload_data,
        size_t *upload_data_size );
} Dispatcher_Entry;


/* prototypes */
extern int dispatch( struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *upload_data,
        size_t *upload_data_size );

#endif

/* vim: set et sm ai ts=2: */
