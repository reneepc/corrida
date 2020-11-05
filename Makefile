pista: pista.c pista.h aleatorio.o
	gcc -pthread pista.c aleatorio.o -o pista -lm -g

aleatorio.o: aleatorio.c aleatorio.h
	gcc -c aleatorio.c

doc/apresentacao.pdf: doc/apresentacao.tex
	cp -r doc/imgs ./imgs
	pdflatex -output-directory=./doc doc/apresentacao.tex
	rm -r ./imgs
	xdg-open doc/apresentacao.pdf
	rm ./doc/*.aux ./doc/*.log ./doc/*.nav ./doc/*toc ./doc/*.snm ./doc/*.out

clean:
	rm *.o pista
