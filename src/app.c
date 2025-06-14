#include "app.h"
#include "interface.h"
#include <ncurses.h>
#include <dirent.h>
#include <stdlib.h>

#define MENU_ITEMS 3

typedef enum {
    STATE_MAIN_MENU,
    STATE_FILE_SELECTION
} AppState;

static AppState current_state = STATE_MAIN_MENU;
static char *app_content = NULL;
static char **file_list = NULL;
static int file_count = 0;
static int file_highlight = 0;

static const char *menu_items[MENU_ITEMS] = {
    "Шифрация текста",
    "Дешифрация с помощью ключа",
    "Дешифрация с помощью метода частотного анализа"
};

static const char *content[MENU_ITEMS] = {
    "шифрования текста...",
    "дешифрования текста...",
    "анализ частот символов..."
};


char *read_file_content(const char *filename) {
    char path[512];
    snprintf(path, sizeof(path), "data/%s", filename);

    FILE *fp = fopen(path, "r");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);
    return buffer;
}

void run_app() {
    init_ui();
    
    int highlight = 0;
    int ch;
    
    app_content = content[highlight];
    draw_menu(menu_items, MENU_ITEMS, highlight);
    draw_content(app_content);

    while ((ch = getch()) != 'q') {
        switch (current_state) {
            case STATE_MAIN_MENU:
                switch (ch) {
                    case KEY_UP:
                        highlight = (highlight - 1 + MENU_ITEMS) % MENU_ITEMS;
                        app_content = content[highlight];
                        break;
                    case KEY_DOWN:
                        highlight = (highlight + 1) % MENU_ITEMS;
                        app_content = content[highlight];
                        break;
                    case 10:
                        handle_menu_selection(highlight);
                        break;
                    case KEY_RESIZE:
                        resize_ui();
                        break;
                };
                break;
        }
        draw_menu(menu_items, MENU_ITEMS, highlight);
        draw_content(app_content);
    }
    
    close_ui();
    free(app_content);
}

void handle_menu_selection(int choice) {
    switch (choice) {
        case 0:
            current_state = STATE_FILE_SELECTION;
            char *filename = show_file_selector();
            if (filename) {
                char *file_text = read_file_content(filename);
                if (file_text) {
                    app_content = file_text;
                } else {
                    draw_content("Ошибка чтения файла.");
                }
                free(filename);
            }
            current_state = STATE_MAIN_MENU;
            break;
        case 1:
            decrypt_text();
            break;
        case 2:
            analyze_frequencies();
            break;
    }
}

void encrypt_text() {
    
}

void decrypt_text() {
}

void analyze_frequencies() {
}