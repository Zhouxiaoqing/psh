CC = gcc
CFLAGS = -O2 -lreadline
CFLAGS_DEBUG = -g -lreadline
TARGET = psh

check-syntax:
		$(CC) -o nul -S $(CHK_SOURCES)

%.o: %.c
		$(CC) $(CFLAGS_DEBUG) -c $<

all:	psh

psh:	psh.o tree.o tokenizer.o parser.o executor.o
		$(CC) $(CFLAGS) -o $(TARGET) *.o

debug:  psh.o tree.o tokenizer.o parser.o executor.o
		$(CC) $(CFLAGS_DEBUG) -o $(TARGET) *.o

clean:
		rm *.o psh
		rm -Rf psh.dSYM

default: all
