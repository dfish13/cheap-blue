CC:=g++
CFLAGS:=-std=c++14 -O3 
PROG_NAME:=cheap-blue

#---
SRC_DIR    := ./src
OBJ_DIR  := ./obj
BIN_DIR    := ./bin
TEST_DIR :=./test
SRC   := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

EXE        := $(BIN_DIR)/$(PROG_NAME)
#-----
$(info VAR="$(OBJ)")
dir_guard=@mkdir -p $(@D)

.PHONY: all  clean

all: $(EXE)
#incase bin directory doesn't exist
$(EXE): $(OBJ) 
	$(dir_guard)
	$(CC) $^ -o $@

#incase obj directory doesn't exist
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp 
	$(dir_guard)
	$(CC) $(CFLAGS) -I ./src/test -c $< -o $@

run: $(EXE)
	$(EXE) 

test: $(EXE) 
	$(EXE) t



# $(TEST): $(TEST_OBJ) | $(OBJ)
# 	$(CC) -o $@ $^ $(FLAGS)

# $(TEST_OBJ): ./test/Test.cpp
# 	$(CC) $(CFLAGS) -c $< -o $@	

# runtest: $(TEST)
# 	./$(TEST)



clean:
	rm -f $(BIN_DIR)/$(PROG_NAME) $(OBJ_DIR)/*.o $(OBJ_DIR)/**/*.o

