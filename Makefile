CC = gcc

CFLAGS = -Wall -Wextra  -Iinclude -pedantic -std=c11 -g

LDLIBS = -lncursesw -lm

TARGET = frequency_analysis_app

SRC = $(wildcard src/*.c)

CC = gcc


# Объектные файлы (.o)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))


# Правило по умолчанию
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

# Правило для компиляции .c в .o
obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj

# Очистка (удаление .o и исполняемого файла)
clean:
	rm -f $(OBJ) $(TARGET)

# Пересобрать проект
rebuild: clean all

# Запуск программы
run: all
	./$(TARGET)

.PHONY: all clean rebuild run