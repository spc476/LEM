
CC = gcc -std=c99 -Wall -Wextra -pedantic
CFLAGS = -g
LDFLAGS = -g
LDLIBS = 

.PHONE: all clean

all : zipf zipw.so zipr.so mz.so
clean:
	$(RM) -rf zipf zipw.so zipr.so mz.so *~

zipf :

zipw.so : zipwlua.c lem.h
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

zipr.so : ziprlua.c lem.h
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

mz.so : mz.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $< -lz
