# CC = gcc
# CFLAGS = -std=c99 -Wall -O2 -fopenmp -lm

main: main.o
	gcc -std=c99 -Wall -O2 -fopenmp -lm -o main main.o

main.o: main.c
	gcc -std=c99 -Wall -O2 -fopenmp -lm -c main.c -o main.o

run:
	./main

clean:
	rm -rf main.o main output.csv

# make
# make run
# make clean