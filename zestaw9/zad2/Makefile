
CC=gcc
CFLAGS=-Wall -std=gnu99 -lpthread 

.PHONY: clean

all: main

main: zad2.c
	$(CC) $< -o zad2 $(CFLAGS)

debug: zad2.c
	$(CC) $< -o $@ $(CFLAGS) -fsanitize=address -lasan  -g 

clean:
	rm zad2