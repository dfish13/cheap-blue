
CC=g++
FLAGS=-std=c++23
EXE=main
UCI=uci

DBG=debug
BIN=bin

SRC=$(wildcard *.cpp)
HDRS=$(wildcard *.h)
OBJ=$(patsubst %.cpp, $(BIN)/%.o, $(filter-out UCI.cpp, $(SRC)))
UCIOBJ=$(patsubst %.cpp, $(BIN)/%.o, $(filter-out Main.cpp, $(SRC)))
DBGOBJ=$(patsubst %.cpp, $(DBG)/%.o, $(SRC))

all: $(EXE) $(UCI)
	
$(EXE): $(OBJ) $(HDRS)
	$(CC) -o $(BIN)/$@ $(OBJ) $(FLAGS)

$(UCI): $(UCIOBJ) $(HDRS)
	$(CC) -o $(BIN)/$@ $(UCIOBJ) $(FLAGS)

$(BIN)/%.o: %.cpp
	@mkdir -p $(BIN)
	$(CC) $(FLAGS) -o $@ -c $^

$(DBG)/%.o: %.cpp
	@mkdir -p $(DBG)
	$(CC) $(FLAGS) -g -o $@ -c $^

$(DBG)/$(EXE): $(DBGOBJ) $(HDRS)
	$(CC) $(FLAGS) -g -o $@ $(DBGOBJ)

run: $(BIN)/$(EXE)
	./$(BIN)/$(EXE)

runtest: $(BIN)/$(EXE)
	./$(BIN)/$(EXE) -t

debug: $(DBG)/$(EXE)
	gdb $(DBG)/$(EXE)

book: book.txt book.py
	python3 book.py

clean:
	rm $(BIN)/*.o $(DBG)/*.o

realclean:
	rm -rf $(BIN) $(DBG)
