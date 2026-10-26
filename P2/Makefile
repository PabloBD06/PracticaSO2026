# Autor1: Pablo Bea Dopazo login: pablo.bea.dopazo
# Autor2: Szymon Arthur Zieba Glaz login: szymon.zieba

CC = gcc
CFLAGS = -Wall -Wextra -g

p2: main.o shellCommands.o memoryCommands.o dynamicList.o
	$(CC) $(CFLAGS) p2.o shellCommands.o dynamicList.o memoryCommands.o -o p2

main.o: main.c shellCommands.h memoryCommands.h dynamicList.h
	$(CC) $(CFLAGS) -c main.c

shellCommands.o: shellCommands.c shellCommands.h dynamicList.h
	$(CC) $(CFLAGS) -c shellCommands.c

memoryCommands.o: memoryCommands.c memoryCommands.h dynamicList.h
	$(CC) $(CFLAGS) -c memoryCommands.c

dynamicList.o: dynamicList.c dynamicList.h
	$(CC) $(CFLAGS) -c dynamicList.c

clean:
	rm -f *.o p2