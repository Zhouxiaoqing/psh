CC = gcc
CFLAGS = -O2 -lreadline
CFLAGS_DEBUG = -g -lreadline -Wall
TARGET = psh

check-syntax:
		$(CC) -o nul -S $(CHK_SOURCES)

%.o: %.c
		$(CC) $(CFLAGS) -c $<

all:	psh

psh:	psh.o tree.o tokenizer.o parser.o executor.o builtins.o
		$(CC) $(CFLAGS) -o $(TARGET) *.o

debug:  psh.o tree.o tokenizer.o parser.o executor.o builtins.o
		$(CC) $(CFLAGS_DEBUG) -o $(TARGET) *.o

clean:
		rm *.o psh
		rm -Rf psh.dSYM

default: all
