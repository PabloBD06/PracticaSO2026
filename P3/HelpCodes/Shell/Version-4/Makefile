CC = gcc
CFLAGS = -g -Wall

shell: Shellv4.c ejemplo.o path.o listasimple.o
	$(CC) -o shell $(CFLAGS) Shellv4.c  ejemplo.o path.o listasimple.o

ejemplo.o: ejemplo.c ejemplo.h path.h
	$(CC) -c $(CFLAGS) ejemplo.c

path.o: path.c path.h listasimple.h
	$(CC) -c $(CFLAGS) path.c

listasimple.o: listasimple.c listasimple.h
	$(CC) -c $(CFLAGS) listasimple.c

clean:
	rm shell *.o
