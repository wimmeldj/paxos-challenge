CC     = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -pedantic


find-pair: chall2.o chall2_lib.o
	$(CC) $(CFLAGS) -o $@ chall2.o chall2_lib.o


chall2.o: chall2.c chall2.h
	$(CC) $(CFLAGS) -c chall2.c

chall2_lib.o: chall2_lib.c chall2_lib.h
	$(CC) $(CFLAGS) -c chall2_lib.c

clean:
	rm chall2.o chall2_lib.o
