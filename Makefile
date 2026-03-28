CC=clang
CFLAGS=-Wall -Wextra -Werror -pedantic
LIBS=-lSDL2
BIN=chip8
SRC=src/main.c
HDR=src/chip8.h

all: $(BIN)

$(BIN): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $(BIN)

debug: CFLAGS += -DDEBUG -ggdb
debug: all

clean:
	rm -r $(BIN)

.PHONY: all debug clean