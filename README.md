# task

Hi,

This directory contains the files associated to my partial solution
to the task.

## To build:

__make server__

Builds the REST server. You can run it then with ./server -p 8080 or any other port number

__make test__

Builds the unit test executable. You can run it then with ./test . It will run all unit tests and provide a result screen. 
Unit tests uses CUnit framework

## Dependencies:

The dependencies are:
 - libmicrohttpd    https://www.gnu.org/software/libmicrohttpd/
   it's a C library that let's you run an HTTP server as part of another app
   it's the server engine that handles HTTP requests
   no particular reason to choose it, it was referenced in the task
   documentation
 - jansson    https://github.com/akheron/jansson
   it's a C library to handle JSON encoding and decoding
   I choose jansson for a very specific reason: it's a thread-safe
   (reentrant) library
   as the server implemented with libmicrohttpd is multithreaded, all
   code (including libraries) must be reentrant to avoid unexpected crashes
   and data corruption because of race conditions

## General overview
The architecture is an HTTP based, multithread server that binds to
a specified port and handles incoming requests.
These requests are REST protocol requests, consisting of HTTP verbs,
with specific CRUD actions on resources.
The mapping between HTTP verbs and CRUD actions is:
 - GET   -->  READ
 - POST  -->  CREATE
 - PUT   -->  UPDATE/CREATE
 - PATCH -->  UPDATE
 - DELETE-->  DELETE

A resource is an object with a type: simply put, it's an entity
For example:  terminals is a resource

The HTTP server is libmicrohttpd
libmicrohttpd knows nothing about REST. The REST functionality and
semantics needs to be implemented by you.
libmicrohttpd has different modes of operations. I configured it
to use a new thread per every new connection it receives. This is standard
configuration, but is far from optimal. IMO, a good libmicrohttpd server
should be configured to use a thread pool. By using it, the server is more
scalable, and can keep resource consumption at an agreed level. Correct
configuration of the thread pool takes time, and is determined empirically by
running many tests.
Entry point to the server is main.c
Besides normal argument parsing with getopts, and the libmicrohttpd
server startup, theres a short custom code in function init_all() 
that populates some terminals, adding them to the "database" (this
"database" is just an array in memory), and starts anew each time the
server is run.

When an HTTP request is received, the URL that represents the resource is
examined and the handling is dispatched to a specific function that knows how
to process the request.
There's a dispatch table and logic implemented in dispatcher.h/dispatcher.c
Basically, for every resource (base URL), there are 5 function pointers
(1 for each HTTP verb).
The code in dispatcher.c is very immature and experimental.
Needs a lot of improvement and refactoring to ease handling
libmicrohttpd responses.
Sorry, I didn't have the time to do this. 
I also couldn't get POST messages work either.
So you only have GET /terminals/1 (or any id)  and GET /terminals
to work.
In general, the REST semantics implemented is weak, and needs more work.

So the processing function called by dispatcher acts like a controller
(sort of) of the resource. It calls functions on the "model" to provide
results.

In particular, the "model" consists of terminals, card types
and transaction types.
Terminals are implemented in terminal.h/terminal.c
Card types are implemented in card_type.h/card_type.c
Transaction types are implemented in transaction_type.h/transaction_type.c

Important functions in terminal.c are:
terminal_to_json() that encodes a terminal "object" into a JSON object.  it's
called by the controller when a request such as
GET /terminals/1  is received
terminal_all_to_json()  that encodes the whole terminals database as an
JSON array. It's called by the controller when a request such as
GET /terminals  is received
terminal_load_json()  that decodes a JSON containing terminal data into
a terminal "object". Validations are done to detect common errors.
it should be called by the controller when a request such as
POST /terminals  is received  (note that the implementation is missing,
I could not make POST handling work in libmicrohttpd, my fault)
terminal_add()  as a terminal to the "database". should be used after
terminal_load_json() in the controller

### Important:
A multithreaded server like this one should not only use reentrant code,
but it should protect with mutexes concurrent access to common structures,
to avoid data corruption and unexpected failures.
Readers can work concurrently with other readers without problem
But writers, should sync on access to common data structures like the
terminals "database", so no more than one writer can modify it at the 
same time.
A writer that's modifiying the "database" should prevent readers
from accessing it as traversal of the array must be protected
I didn't have the time to implement these mutexes.

Other missing things that you should expect in a production ready server is
security. This implementation doesn't protect the resources, nor handle
https traffic. 
The server should also be made more robust, becoming a deamon, detaching
from it's controlling terminal / process group.
It should also use wait() system call after starting libmicrohttpd so
it remains running until interrupted. I would suggest also to implement
signal handling.
Other suggestion to consider is to enable via a secure, controlled resource,
a set of commands to be given to the server itself. For example, to gracefully
shutdown. Or to refresh a configuration file. Care should be taken, this
can be a double edged sword.

A note about acceptance tests:
At this time acceptance tests are not automated. Testing was done manually using __curl__.

