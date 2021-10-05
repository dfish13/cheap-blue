
CC=g++
FLAGS=-std=c++14
EXE=main

DBG=debug
BIN=bin

APPDIR=../

SRC=$(wildcard *.cpp)
HDRS=$(wildcard *.h)
OBJ=$(patsubst %.cpp, $(BIN)/%.o, $(SRC))
DBGOBJ=$(patsubst %.cpp, $(DBG)/%.o, $(SRC))

all: $(EXE)
	
$(EXE): $(OBJ) $(HDRS)
	$(CC) -o $(BIN)/$@ $(OBJ) $(FLAGS)

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

clean:
	rm $(BIN)/*.o $(DBG)/*.o

realclean:
	rm -rf $(BIN) $(DBG)
