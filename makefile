
TEST=test.jpl
FLAGS=-t

INCLUDE = ../include/
CXX=clang++
# CXXFLAGS=-Og -std=c++17 -Werror -Wall -fsanitize=address,undefined -fno-sanitize-recover=address,undefined
CXXFLAGS=-Og -std=c++17 -Wall -fsanitize=address,undefined -fno-sanitize-recover=address,undefined

all: run

compile: compiler.o

typechecker.o: 
	$(CXX) $(CXXFLAGS) -c lib/typechecker.cpp -o src/typechecker.o

parser.o: 
	$(CXX) $(CXXFLAGS) -c lib/parser.cpp -o src/parser.o

lexer.o: 
	$(CXX) $(CXXFLAGS) -c lib/lexer.cpp -o src/lexer.o

compiler.o:
	$(CXX) $(CXXFLAGS) -c lib/compiler.cpp -o src/compiler.o

a.out: lexer.o parser.o typechecker.o compiler.o
	$(CXX) $(CXXFLAGS) src/*.o -o a.out

run:
	./a.out $(FLAGS) $(TEST)



janker: lexer.o parser.o compiler.o
	$(CXX) $(CXXFLAGS) src/compiler.o src/parser.o src/lexer.o -o src/jank

jank:
	./src/jank $(FLAGS) $(TEST)

clean:
	rm -f src/*.o src/a.out src/jank.out