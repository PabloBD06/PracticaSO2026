# Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
# Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

CC = gcc
CFLAGS = -Wall -Wextra -g

p1: p1.o comandosp1.o dynamicList.o
	$(CC) $(CFLAGS) p1.o comandosp1.o dynamicList.o -o p1

p1.o: p1.c comandosp1.h
	$(CC) $(CFLAGS) -c p1.c

comandosp1.o: comandosp1.c comandosp1.h dynamicList.h
	$(CC) $(CFLAGS) -c comandosp1.c

dynamicList.o: dynamicList.c dynamicList.h
	$(CC) $(CFLAGS) -c dynamicList.c

clean:
	rm -f *.o p1