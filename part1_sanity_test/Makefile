# Makefile for userspace temperature test program
# ATU Sligo - Operating Systems Interfacing Lab 8

CC = gcc
CFLAGS = -Wall -Wextra

temp_test: temp_test.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f temp_test

.PHONY: clean