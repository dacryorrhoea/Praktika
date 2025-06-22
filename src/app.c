#include "app.h"
#include "crypto.h"
#include "freq_analysis.h"
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
static char* curr_filename;
static char* app_content = NULL;
static char** file_list = NULL;
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

void run_app() {
    init_ui();
    
    int highlight = 0;
    int content_offset = 0;
    
    app_content = "";
    draw_menu(menu_items, MENU_ITEMS, highlight);
    draw_content("", app_content, 10);

    int ch;
    while ((ch = getch()) != 'q') {
        switch (current_state) {
            case STATE_MAIN_MENU:
                switch (ch) {
                    case KEY_UP:
                        highlight = (highlight - 1 + MENU_ITEMS) % MENU_ITEMS;
                        break;
                    case KEY_DOWN:
                        highlight = (highlight + 1) % MENU_ITEMS;
                        break;
                    case 'j':
                        if (app_content) content_offset += 1;
                        break;
                    case 'k':
                        if (app_content && content_offset > 0) content_offset -= 1;
                        break;
                    case 10:
                        handle_menu_selection(highlight);
                        content_offset = 0;
                        break;
                    case KEY_RESIZE:
                        resize_ui();
                        break;
                };
                break;
        }
        draw_menu(menu_items, MENU_ITEMS, highlight);
        draw_content(curr_filename?curr_filename:"", app_content, content_offset);
    }
    
    close_ui();
    free(app_content);
    free(curr_filename);
}

void handle_menu_selection(int choice) {
    switch (choice) {
        case 0: {
            current_state = STATE_FILE_SELECTION;
            char* filename = show_file_selector("Выберите файл для шифрации:\n");
            if (filename) {
                char* encrypton_file = encryption(filename);
                char* file_text = read_file_content(encrypton_file);
                if (file_text) {
                    curr_filename = encrypton_file;
                    app_content = file_text;
                } else {
                    draw_content("", "Ошибка чтения файла.", 0);
                }
                free(filename);
            }
            current_state = STATE_MAIN_MENU;
            break;
        }
        case 1: {
            current_state = STATE_FILE_SELECTION;
            char* shifr_filename = show_file_selector("Выберите файл для дешифрации:\n");
            char* key_filename = show_file_selector("Выберите с ключом для шифра:\n");
            if (shifr_filename && key_filename) {
                char* decrypton_file = decryption_with_key(shifr_filename, key_filename);
                char* file_text = read_file_content(decrypton_file);
                if (file_text) {
                    curr_filename = decrypton_file;
                    app_content = file_text;
                } else {
                    draw_content("", "Ошибка чтения файла.", 0);
                }
                free(shifr_filename);
                free(key_filename);
            }
            current_state = STATE_MAIN_MENU;
            break;
        }
        case 2: {
            current_state = STATE_FILE_SELECTION;
            char* shifr_filename = show_file_selector("Выберите файл для дешифрации:\n");
            if (shifr_filename) {
                char* decrypton_file = decryption_without_key(shifr_filename);
                char* file_text = read_file_content(decrypton_file);
                if (file_text) {
                    curr_filename = decrypton_file;
                    app_content = file_text;
                } else {
                    draw_content("", "Ошибка чтения файла.", 0);
                }
                free(shifr_filename);
            }
            current_state = STATE_MAIN_MENU;
            break;
        }
    }
}