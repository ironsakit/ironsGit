irongit.exe: index.c main.c objects.c utils/*.c
	gcc -Wall index.c main.c objects.c utils/*.c -o irongit.exe -lz