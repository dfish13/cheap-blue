CC:=g++
CFLAGS:=-std=c++14 -O3 
PROG_NAME:=cheap-blue

#---
# SRC_DIR    := ./src
# OBJ_DIR  := ./obj
# BIN_DIR    := ./bin
# TEST_DIR :=./test
# SRC   := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/**/*.cpp)
# OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
# TEST_OBJ := $(OBJ_DIR)/Test.o

# EXE        := $(BIN_DIR)/$(PROG_NAME)
# TEST_EXE       :=$(BIN_DIR)/test-$(PROG_NAME)

#-----
MODULES   := test ui
SRC_DIR   := $(addprefix src/,$(MODULES))
BUILD_DIR := $(addprefix obj/,$(MODULES))

SRC       := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
OBJ       := $(patsubst src/%.cpp,obj/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR))


vpath %.cpp $(SRC_DIR)

define make-goal
$1/%.o: %.cpp
    $(CC) $(INCLUDES) -c $$< -o $$@
endef

.PHONY: all checkdirs clean

#all: $(EXE)
all: checkdirs build/test.exe

build/test.exe: $(OBJ)
	$(CC) $^ -o $@

checkdirs: $(BUILD_DIR)

#incase bin directory doesn't exist
# $(EXE): $(OBJ) | $(BIN_DIR)
# 	$(CC) $^ -o $@

#incase obj directory doesn't exist
# $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
# 	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

run:
	./$(PROG_NAME)

# $(TEST): $(TEST_OBJ) | $(OBJ)
# 	$(CC) -o $@ $^ $(FLAGS)

# $(TEST_OBJ): ./test/Test.cpp
# 	$(CC) $(CFLAGS) -c $< -o $@	

# runtest: $(TEST)
# 	./$(TEST)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@


clean:
	rm -f $(BIN_DIR)/$(PROG_NAME) $(OBJ_DIR)/*.o

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))
