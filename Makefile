CC=gcc
CFLAGS=-I./src -Wall -Wextra -O2 -std=c99 -march=native
LDFLAGS=-lSDL2 -lm
SRC=src/main.c src/particle.c src/renderer.c src/utils.c
OBJ=$(SRC:.c=.o)
TARGET=particles-demo

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all run clean