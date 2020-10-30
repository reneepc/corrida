pista: pista.c pista.h
	gcc -pthread pista.c -o pista 

aleatorio.o: aleatorio.c aleatorio.h
	gcc -c aleatorio.c

clean:
	rm *.o pista
