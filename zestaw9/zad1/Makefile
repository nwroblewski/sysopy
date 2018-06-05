
CC=gcc
CFLAGS=-Wall -std=gnu99 -lpthread 

.PHONY: clean

all: main

main: zad1.c
	$(CC) $< -o zad1 $(CFLAGS)

debug: zad1.c
	$(CC) $< -o $@ $(CFLAGS) -fsanitize=address -lasan  -g 

clean:
	rm zad1