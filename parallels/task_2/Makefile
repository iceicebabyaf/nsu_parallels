main: main.o
	gcc -std=c99 -Wall -O2 -fopenmp -o main main.o -lm

main.o: main.c
	gcc -std=c99 -Wall -O2 -fopenmp -c main.c -o main.o

run:
	./main

clean:
	rm -rf main.o main output.csv