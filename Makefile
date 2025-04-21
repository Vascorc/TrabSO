CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/process.c src/scheduler_dynamic.c src/scheduler_static.c src/utils.c
OBJ = $(SRC:.c=.o)
BIN = bin/probsched

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -f $(BIN) src/*.o
