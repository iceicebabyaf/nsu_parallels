CXX = g++
CXXFLAGS = -Wall -O2 -fopenmp -pthread

EXEC = mainP
OBJ = mainP.o

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJ)

$(OBJ): mainP.cpp
	$(CXX) $(CXXFLAGS) -c mainP.cpp -o $(OBJ)

run: $(EXEC)
	./$(EXEC)

clean:
	rm -rf $(OBJ) $(EXEC) pows.txt sins.txt sqrts.txt
