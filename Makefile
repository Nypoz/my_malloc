CC     := gcc
CFLAGS := -Wall -Wextra -g -O0
BIN    := test_malloc

all: $(BIN)

$(BIN): src/mymalloc.c src/test.c
	$(CC) $(CFLAGS) -o $@ $^

run: all
	./$(BIN)

clean:
	rm -f $(BIN)

.PHONY: all run clean
