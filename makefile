
src=BitBoard.cpp Board.cpp Util.cpp

# Transforms source file names into object file names
obj=$(src:.cpp=.o)


CC=g++
FLAGS=-std=c++14 -O3
EXE=main
TEST=test

all: $(EXE) $(TEST)
	
$(EXE): Main.o $(obj)
	$(CC) -o $@ $^ $(FLAGS)

%.o: %.cpp
	$(CC) $(FLAGS) -c $^

run:
	./$(EXE)

$(TEST): Test.o $(obj)
	$(CC) -o $@ $^ $(FLAGS)

runtest:
	./$(TEST)

clean:
	rm *.o $(EXE) $(TEST)
