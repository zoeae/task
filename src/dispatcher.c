/*
 * dispatcher.c
 *
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dispatcher.h"
#include "terminal.h"


int terminals_get_handler( struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *upload_data,
        size_t *upload_data_size ) {
  static char *terminal_not_found = "{\n\
\"error\": \"not found\",\n\
\"error_description\": \"longer description, human-readable\"\n\
}";

  static char *unspecified_error = "{\n\
\"error\": \"unspecified error\",\n\
\"error_description\": \"longer description, human-readable\"\n\
\"error_uri\": \"URI to a detailed error description on the API developer website\"\n\
}";

  struct MHD_Response *response;
  int ret;
  char *p;
  int resource_id;

  fprintf(stderr, "INSIDE terminals_get_handler\n");

   /* get the resource name */
  if (strncmp(url, "/terminals/", strlen("/terminals/")) == 0) {
    /* now get the resource id */
    resource_id = atoi(url + strlen("/terminals/"));
    fprintf(stderr, "%s URL=%s resource=%d\n", method, url, resource_id);
    /* get the data */
    Terminal_Data *t = terminal_find_by_id(resource_id);
    if (t == NULL) {
      fprintf(stderr, "terminal not found");
      /* return error */
      response = MHD_create_response_from_buffer(
		      strlen(terminal_not_found),
                      (void*) terminal_not_found,
                      MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response(connection,
                    404,
                    response);
    } else {
      fprintf(stderr, "terminal found ");
      p = terminal_to_json(t);
      fprintf(stderr, "JSON=%s\n", p);
      response = MHD_create_response_from_buffer(strlen(p),
                  (void*) p,
                  MHD_RESPMEM_MUST_FREE);
      ret = MHD_queue_response(connection,
                    MHD_HTTP_OK,
                    response);
    }
  } else if (strcmp(url, "/terminals") == 0) {
    fprintf(stderr, "retrieve all terminals\n");
    p = terminal_all_to_json();
    fprintf(stderr, "JSON=%s\n", p);
    response = MHD_create_response_from_buffer(strlen(p),
                  (void*) p,
                  MHD_RESPMEM_MUST_FREE);
    ret = MHD_queue_response(connection,
                    MHD_HTTP_OK,
                    response);
  } else {
    /* return error */
    response = MHD_create_response_from_buffer(strlen(unspecified_error),
                  (void*) unspecified_error,
		  MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection,
                    400,
                    response);
  }

  MHD_destroy_response(response);

  return ret;
}

static Dispatcher_Entry Dispatch_Table[] = {
  { "/terminals",
     { terminals_get_handler, 
       NULL, 
       NULL, 
       NULL, 
       NULL 
     }
  },
  { 0, { NULL, NULL, NULL, NULL } }
};


static int method_to_idx(const char *method) {
  int i = -1;

  if (strcmp(method, "GET") == 0) {
    i = 0;
  } else if (strcmp(method, "POST") == 0) {
    i = 1;
  } else if (strcmp(method, "PUT") == 0) {
    i = 2;
  } else if (strcmp(method, "PATCH") == 0) {
    i = 3;
  } else if (strcmp(method, "DELETE") == 0) {
    i = 4;
  }
  return i;
}

int dispatch( struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *upload_data,
        size_t *upload_data_size ) {
  int i;
  int idx;
  char *p;
  char url_base[BUFSIZ];

  /* get the base URL */
  strncpy(url_base, url, BUFSIZ);
  url_base[BUFSIZ - 1] = '\0';
  if ((p = strrchr(url_base, '/')) != NULL) {
    if (p != url_base) {
      *p = '\0';
    }
  }

  /* search the dispatch table */
  for (i = 0; Dispatch_Table[i].url != NULL; i++ ) {
    /* url should match */
    if (strncmp(Dispatch_Table[i].url, url_base, strlen(Dispatch_Table[i].url)) == 0 ) {
      /* get http verb */
      idx = method_to_idx(method);
      if (idx == -1) {
	/* invalid verb */
        return MHD_NO;
      }
      if (Dispatch_Table[i].dispatch_function[idx] == NULL) {
	/* no function configured for this verb */
        return MHD_NO;
      }
      /* call the function */
      return (Dispatch_Table[i].dispatch_function[idx])(
         connection,
         url,
         method,
         upload_data,
         upload_data_size
	 );
    }
  }
  return MHD_NO;
}



/* vim: set et sm ai ts=2: */
