CXX = g++
CXXFLAGS = -Wall -O2 -fopenmp -pthread

main: main.o
	$(CXX) $(CXXFLAGS) -o main main.o

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

run:
	./main

clean:
	rm -rf main.o main output.csv
