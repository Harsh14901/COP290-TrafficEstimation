CC		= g++
C_FLAGS = -g -Wall

BIN		= bin
SRCS	= src/*.cpp
PROG = bin/main
SRC = src
INCLUDE	:= include
LIB		:= lib


OPENCV = $(shell pkg-config opencv4 --cflags --libs 2>/dev/null || pkg-config opencv --cflags --libs 2>/dev/null)
LIBS = $(OPENCV)

	
EXECUTABLE = main
all: $(BIN)/$(EXECUTABLE)

clean:
	$(RM) $(BIN)/$(EXECUTABLE)

run: all
	./$(BIN)/$(EXECUTABLE) $(args)
$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp ./src/util/*.cpp
	
	$(CC) $(C_FLAGS) -I$(INCLUDE) -I ./src/ -L$(LIB) $^ -o $@ $(LIBS)
tar:
	tar -zcvf 2019CS10431_2019CS50443_ass1_part1.tar.gz --directory=./ --exclude-vcs --exclude-vcs-ignores --exclude=*.tar.gz --exclude=".vscode/*" *
