CC = gcc
flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: forkFFT

myexpaned: forkFFT.o
	$(CC) $(flags) -o $@ $^

%.o: %.c
	$(CC) $(flags) -c -o $@ $<

clean:
	rm -rf *.o forkFFT