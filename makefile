
CC=g++
FLAGS=-std=c++14 -O3
EXE=main

BIN=bin

SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp, $(BIN)/%.o, $(SRC))

all: $(EXE) $(TEST)
	
$(EXE): $(OBJ)
	$(CC) -o $(BIN)/$@ $^ $(FLAGS)

$(BIN)/%.o: %.cpp
	$(CC) $(FLAGS) -o $@ -c $^

run:
	./$(BIN)/$(EXE)

runtest:
	./$(BIN)/$(EXE) -t

clean:
	rm $(BIN)/*.o

realclean:
	rm -rf $(BIN)
