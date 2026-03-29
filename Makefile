all: irongit.exe

irongit.exe: main.c utils/*.c
	gcc -Wall main.c utils/*.c -o irongit.exe -lz -lws2_32