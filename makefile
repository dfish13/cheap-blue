CC:=g++
CFLAGS:=-std=c++14 -O3 
PROG_NAME:=cheap-blue

SRC_DIR    := ./src
OBJ_DIR  := ./obj
BIN_DIR    := ./bin
SRC   := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

EXE        := $(BIN_DIR)/$(PROG_NAME)

dir_guard=@mkdir -p $(@D)

.PHONY: all  clean test

all: $(EXE)

#incase bin directory doesn't exist
$(EXE): $(OBJ) 
	$(dir_guard)
	$(CC) $^ -o $@

#incase obj directory doesn't exist
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	$(dir_guard)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXE)
	$(EXE) 

test: $(EXE) 
	$(EXE) t

clean:
	rm -f $(BIN_DIR)/$(PROG_NAME) $(OBJ_DIR)/*.o $(OBJ_DIR)/**/*.o

