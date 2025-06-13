CC = gcc

CFLAGS = -Wall -Wextra -pedantic -std=c11 -g

LDLIBS = -lncursesw -lm

TARGET = frequency_analysis_app

SRC = src/main.c

# Объектные файлы (.o)
# OBJ = $(SRC:.c=.o)
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