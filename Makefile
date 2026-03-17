CC = gcc
CFLAGS = -Wall

SRC = $(wildcard *.c)

irongit.exe:
	$(CC) $(CFLAGS) $(SRC) -o irongit.exe

clean:
	del irongit.exe