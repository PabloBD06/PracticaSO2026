CC = gcc
CFLAGS = -Wall -Wextra -g

shell: main.o comandosp1.o dynamicList.o
	$(CC) $(CFLAGS) main.o comandosp1.o dynamicList.o -o shell

main.o: main.c comandosp1.h
	$(CC) $(CFLAGS) -c main.c

comandosp1.o: comandosp1.c comandosp1.h dynamicList.h
	$(CC) $(CFLAGS) -c comandosp1.c

dynamicList.o: dynamicList.c dynamicList.h
	$(CC) $(CFLAGS) -c dynamicList.c

clean:
	rm -f *.o shell