#
#

CC=gcc
CFLAGS=-I.
DEPS = card_type.h transaction_type.h terminal.h dispatcher.h
OBJ = card_type.o transaction_type.o terminal.o main.o dispatcher.o
LIBS = libjansson.a libmicrohttpd.a

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -l microhttpd -l jansson

test: card_type.o transaction_type.o terminal.o test.o
	$(CC) -o $@ $^ $(CFLAGS) -lcunit -ljansson

clean: rm server $(OBJ)
