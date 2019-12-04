/*
 *  main.c
 *
 */

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include  <signal.h>
#include  <errno.h>
#include  <time.h>


#include "microhttpd.h"
#include "terminal.h"
#include "dispatcher.h"

#ifndef FILENAME_MAX
#define  FILENAME_MAX  250
#endif

#define DEFAULT_SERVER_PORT  8080

/* variables globales */
char  *pgm_name;                /* program name */
char  *log_fname;               /* a file to use to log debug messages and errors */
int   server_port_number = DEFAULT_SERVER_PORT; /* this is the port for the server to receive connections */

/* to explain command use */
static char  *use[] = {
  "",
  "Options: -l  log file name (default is stdout)",
  "         -p  tcp binding port (default is 8080)",
  "         -V  tool version number",
  (char *) NULL
};


/* prototypes */
static int parse_cmd_line( int argc, char *argv[] );
static void usage( void );
static void cleanup( void );
static int init_all( void );



/*
 * processing callback funtion for new data received
 */
static int ahc_handler(void *cls,
        struct MHD_Connection *connection,
        const char *url,
        const char *method,
        const char *version,
        const char *upload_data,
        size_t *upload_data_size,
        void **ptr) {
  static int dummy;
  const char * page = cls;
  struct MHD_Response * response;
  int ret;
  int resource_id;
  char *p;

  fprintf(stderr, "Handler %s URL=%s\n", method, url);

  if (&dummy != *ptr) {
      /* The first time only the headers are valid,
         do not respond in the first round... */
      *ptr = &dummy;
      return MHD_YES;
  }

#if 0
  if (0 != strcmp(method, "GET")) {
    return MHD_NO; /* unexpected method */
  }

  /* these are all GETs */
#endif

  fprintf(stderr, "Before dispatch %s URL=%s\n", method, url);
  ret = dispatch(connection, url, method, upload_data, upload_data_size);
  fprintf(stderr, "After dispatch %s URL=%s  ret=%d\n", method, url, ret);
  return ret;
}


int main( int argc, char *argv[] ) {
  struct MHD_Daemon *d;

  /* parse command line arguments */
  if ( parse_cmd_line( argc, argv ) == 0 ) {
    usage();
    exit( EXIT_FAILURE );
  }

  /* become a daemon
   * a good server will take extra steps to become a daemon,
   * to dettach from any controlling terminal, etc
   * so no signals get triggered in situations that will stop the process
   */

  /* init all */
  if ( init_all() == 0 ) {
    exit( EXIT_FAILURE );
  }

  /*
   * add program termination routine
   * cleanup will be executed before exiting the process
   */
  if ( atexit( cleanup ) != 0 ) {
    exit( EXIT_FAILURE );
  }

  /* start the libmicrohttpd server
   * i think the best configuration is to use a thread pool to
   * handle requests. this will help with scalability, to sustain
   * a certain processing level. this mode is like the "reactor" pattern
   */
  d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                  server_port_number,
                  NULL,
                  NULL,
                  &ahc_handler,
                  NULL,
                  MHD_OPTION_END);

  if (d == NULL) {
    return 1;
  }

  /* just wait for some input to stop the server
   * this is for test only
   * a real server would use the wait() system call
   * to block this thread while the MDD threads keep running
   * it's common also to capture signals to use as simple IPC for
   * the server
   */
  (void) getc(stdin);

  /* stop the libmicrohttpd server */
  MHD_stop_daemon(d);

  exit( EXIT_SUCCESS );
}


/*
 *  init all
 */
static int init_all( void ) {
  /*
   * this is the typical place where configuration files can be read to
   * populate structures
   * or can restore saved state from another run
   * for example: the sequence number for the terminals table
   * empty for now
   */

  /* add some terminals to the terminals db so it's not empty
   * as the code for POST is not ready yet, this is a way to
   * have some data to test the other code
   */
  bool st;

  Terminal_Data t;
  terminal_init_data(&t);
  terminal_add_card_type(&t, "Visa" );
  terminal_add_transaction_type(&t, "Credit" );
  terminal_add(&t);

  char *p = "{\n\
 \"id\": 99,\n\
 \"CardType\": [\n\
  \"Visa\",\n\
  \"MasterCard\",\n\
  \"EFTPOS\"\n\
 ],\n\
 \"TransactionType\": [\n\
  \"Cheque\",\n\
  \"Credit\"\n\
 ]\n\
}";
  st = terminal_load_json(&t, p);
  if (st) {
    terminal_add(&t);
  } else {
    fprintf(stderr, "error loading from JSON\n");
  }

  return 1;
}


/*
 *  cleanup
 */
static void cleanup( void )
{
  /* this function is set as a termination funtion to be called when program ends
   * a server can save state by persisting data to a file for example
   * for example: the sequence number for the terminals table
   */
  return;
}


/*
 *  parsing of command line arguments
 */
static int parse_cmd_line( int argc, char *argv[] ) {
  int    c;
  extern  char  *optarg;

  /* get plain program name */
  if ( (pgm_name = strrchr( argv[0], '/' )) == (char *) NULL ) {
    pgm_name = argv[0];
  }
  else {
    pgm_name++;
  }

  log_fname = (char *) NULL;
  while ( (c = getopt( argc, argv, "l:p:V" )) != EOF ) {
    switch ( c ) {
      case 'l':
        log_fname = optarg;
        break;

      case 'p':
        server_port_number = atoi(optarg);
        break;
      
      case 'V':
        fprintf( stderr, "%s: REST Server\n",
          pgm_name );
        break;

      case '?':
        return 0;
        /* NOTREACHED */
        break;
    }
  }

  return 1;
}


/*
 *  command usage
 */
static void usage( void ) {
  int    i;

  fprintf( stderr, "%s\n", pgm_name );
  for ( i = 0; use[i] != (char *) NULL; i++ ) {
    fprintf( stderr, "%s\n", use[i] );
  }
}
