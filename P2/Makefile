# Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
# Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

CC = gcc
CFLAGS = -Wall -Wextra -g

p2: p2.o comandos.o dynamicList.o
	$(CC) $(CFLAGS) p2.o comandos.o dynamicList.o -o p2

p2.o: p2.c comandos.h
	$(CC) $(CFLAGS) -c p2.c

comandos.o: comandos.c comandos.h dynamicList.h
	$(CC) $(CFLAGS) -c comandos.c

dynamicList.o: dynamicList.c dynamicList.h
	$(CC) $(CFLAGS) -c dynamicList.c

clean:
	rm -f *.o p2