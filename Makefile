BIN = ./bin
SRC = ./src

all: make_directories $(BIN)
	g++ -Wall -Werror -ansi -pedantic $(SRC)/rshell.cpp -o $(BIN)/rshell
	g++ -Wall -Werror -ansi -pedantic $(SRC)/ls.cpp -o $(BIN)/ls
	g++ -Wall -Werror -ansi -pedantic $(SRC)/cp.cpp -o $(BIN)/cp
rshell: 
	g++ -Wall -Werror -ansi -pedantic $(SRC)/rshell.cpp -o $(BIN)/rshell
	g++ -Wall -Werror -ansi -pedantic $(SRC)/ls.cpp -o $(BIN)/ls
	g++ -Wall -Werror -ansi -pedantic $(SRC)/cp.cpp -o $(BIN)/cp

.PHONY make_directories: $(BIN)/

$(BIN)/:
	mkdir -p $@

