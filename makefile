
CC=g++
FLAGS=-std=c++14 -O3
EXE=main

DBG=debug
BIN=bin

SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp, $(BIN)/%.o, $(SRC))
DBGOBJ=$(patsubst %.cpp, $(DBG)/%.o, $(SRC))

all: $(EXE)
	
$(EXE): $(OBJ)
	$(CC) -o $(BIN)/$@ $^ $(FLAGS)

$(BIN)/%.o: %.cpp
	@mkdir -p $(BIN)
	$(CC) $(FLAGS) -o $@ -c $^

$(DBG)/%.o: %.cpp
	@mkdir -p $(DBG)
	$(CC) $(FLAGS) -g -o $@ -c $^

$(DBG)/$(EXE): $(DBGOBJ)
	$(CC) $(FLAGS) -g -o $@ $^

run: $(BIN)/$(EXE)
	./$(BIN)/$(EXE)

runtest: $(BIN)/$(EXE)
	./$(BIN)/$(EXE) -t

debug: $(DBG)/$(EXE)
	gdb $(DBG)/$(EXE)

clean:
	rm $(BIN)/*.o $(DBG)/*.o

realclean:
	rm -rf $(BIN) $(DBG)
