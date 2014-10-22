BIN = ./bin
SRC = ./src

all: make_directories $(BIN)
	g++ -Wall -Werror -ansi -pedantic $(SRC)/rshell.cpp -o $(BIN)/rshell

rshell: 
	g++ -Wall -Werror -ansi -pedantic $(SRC)/rshell.cpp -o $(BIN)/rshell

.PHONY make_directories: $(BIN)/

$(BIN)/:
	mkdir -p $@

