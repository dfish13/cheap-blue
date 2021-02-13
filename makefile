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


.PHONY: all  clean

all: $(EXE)
#incase bin directory doesn't exist
$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $^ -o $@

#incase obj directory doesn't exist
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run:
	./$(PROG_NAME)

# $(TEST): $(TEST_OBJ) | $(OBJ)
# 	$(CC) -o $@ $^ $(FLAGS)

# $(TEST_OBJ): ./test/Test.cpp
# 	$(CC) $(CFLAGS) -c $< -o $@	

# runtest: $(TEST)
# 	./$(TEST)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $(@D)




clean:
	rm -f $(BIN_DIR)/$(PROG_NAME) $(OBJ_DIR)/*.o

