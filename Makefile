CFLAGS= -g -Wall -pedantic -D_BSD_SOURCE

all: snp schat

snp: snp.o listas.o functions.o
	gcc $(CFLAGS) -o snp snp.o listas.o functions.o

schat:
	gcc $(CFLAGS) -o schat schat.c
		
snp.o: snp.c functions.h listas.h
	gcc $(CFLAGS) -c snp.c

listas.o: listas.c listas.h
	gcc $(CFLAGS) -c listas.c

functions.o: functions.c functions.h
	gcc $(CFLAGS) -c functions.c

clean:
	rm -f *.o snp
	rm -f *.o schat

