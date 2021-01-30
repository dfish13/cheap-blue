# Collects all source files ending in .cpp
src=$(wildcard *.cpp)

# Transforms source file names into object file names
obj=$(src:.cpp=.o)


CC=g++
FLAGS=-std=c++14 -O3
exe=main

$(exe): $(obj)
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) $(FLAGS) -c $<

run:
	./$(exe)

clean:
	rm $(obj) $(exe)
