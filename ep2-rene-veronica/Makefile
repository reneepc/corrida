ep2: ep2.c ep2.h aleatorio.o
	gcc -pthread ep2.c aleatorio.o -o ep2 -lm -g

aleatorio.o: aleatorio.c aleatorio.h
	gcc -c aleatorio.c

clean:
	rm *.o ep2
