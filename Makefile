CC = gcc
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wno-unused -Wfloat-equal -Wdouble-promotion -Wformat-overflow=2 -Wformat=2 -Wnull-dereference -Wno-unused-result -Wmissing-include-dirs -Wswitch-default

example:
	$(CC) $(CFLAGS) -I. cunit.c example/suite1.c example/suite2.c -o test_example

.PHONY: all example
