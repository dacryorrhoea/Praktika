CC = gcc

CFLAGS = -Wall -Wextra  -Iinclude -pedantic -std=c11 -g

LDLIBS = -lncursesw -lm

TARGET = frequency_analysis_app

SRC = $(wildcard src/*.c)

CC = gcc

OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))


all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -f $(OBJ) $(TARGET)

rebuild: clean all

run: all
	./$(TARGET)

.PHONY: all clean rebuild run