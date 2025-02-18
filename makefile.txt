
TEST=test.jpl
FLAGS=-t
INCLUDE = ../include/
CXX=clang++
# CXXFLAGS=-Og -std=c++17 -Werror -Wall -fsanitize=address,undefined -fno-sanitize-recover=address,undefined
# CXXFLAGS=-Og -std=c++20 -Wall -fsanitize=address,undefined -fno-sanitize-recover=address,undefined
CXXFLAGS=-Og -std=c++23 -Wall -fsanitize=address,undefined -fno-sanitize-recover=address,undefined
# CXXFLAGS=-O3 -std=c++20 -Wall -fsanitize=address,undefined -fno-sanitize-recover=address,undefined

### LLVM IR VARIABLES
LEVEL = ./lib # just '.' ?
PROJECT_NAME = JANK
LLVM_SRC_ROOT = 



a.out: lexer.o parser.o typechecker.o driver.o
	$(CXX) $(CXXFLAGS) src/*.o -o JANK

all: run

compile: driver.o

typechecker.o:  # perhaps lib -> src && src -> ??? depends on prod
	$(CXX) $(CXXFLAGS) -c lib/typechecker.cpp -o src/typechecker.o

parser.o: 
	$(CXX) $(CXXFLAGS) -c lib/parser.cpp -o src/parser.o

lexer.o: 
	$(CXX) $(CXXFLAGS) -c lib/lexer.cpp -o src/lexer.o

driver.o:
	$(CXX) $(CXXFLAGS) -c lib/driver.cpp -o src/driver.o

run:
	./JANK $(FLAGS) $(TEST)

comp:
	./a.out $(FLAGS) $(TEST) > eval/mine.txt
	./jplc-macos $(FLAGS) $(TEST) > eval/def.txt
	diff -c eval/def.txt eval/mine.txt

janker: lexer.o parser.o driver.o
	$(CXX) $(CXXFLAGS) src/driver.o src/parser.o src/lexer.o -o src/jank

jank:
	a.out

clean:
	rm -f src/*.o JANK src/jank.out