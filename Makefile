pista: pista.c pista.h aleatorio.o
	gcc -pthread pista.c aleatorio.o -o pista -lm -g

aleatorio.o: aleatorio.c aleatorio.h
	gcc -c aleatorio.c

clean:
	rm *.o pista
