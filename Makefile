
CC = gcc -std=c99 -Wall -Wextra -pedantic
CFLAGS = -g
LDFLAGS = -g
LDLIBS = 

zipf :

zipw.so : zipwlua.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<
