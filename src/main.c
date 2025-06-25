#include "crypto.h"
#include "utils.h"
#include "interface.h"
#include <ncurses.h>
#include <stdlib.h>

#define FILE_ERROR_MSG "files open error"

#define UI_TEXT_MSG_CRYPT "Выберите файл для дешифрации:\n"
#define UI_TEXT_MSG_DECRYPT "Выберите файл для дешифрации:\n"
#define UI_TEXT_MSG_KEY "Выберите файл с ключом:\n"

#define MENU_ITEMS 3
static const char *menu_items[MENU_ITEMS] = {
    "Шифрация текста",
    "Дешифрация с помощью ключа",
    "Дешифрация с помощью метода частотного анализа"
};

static char* curr_filename = NULL;
static char* app_content = NULL;
static int content_offset = 0;
static int highlight = 0;

void load_and_display_file(char* filename) {
    char* result = read_file_content(filename);
    if (!result) DIE(FILE_ERROR_MSG);
    
    free(curr_filename);
    free(app_content);
    
    curr_filename = filename;
    app_content = result;

    content_offset = 0;
}

void encrypt_file() {
    char* filename = show_file_selector(UI_TEXT_MSG_CRYPT);
    if (!filename) DIE(FILE_ERROR_MSG);
    
    char* encrypted_file = encryption(filename);
    load_and_display_file(encrypted_file);

    free(filename);
}

void decrypt_with_key() {
    char* enc_file = show_file_selector(UI_TEXT_MSG_DECRYPT);
    if (!enc_file) DIE(FILE_ERROR_MSG);
    
    char* key_file = show_file_selector(UI_TEXT_MSG_KEY);
    if (!key_file) {
        free(enc_file);
        DIE(FILE_ERROR_MSG);
    }
    
    char* decrypted = decryption_with_key(enc_file, key_file);
    load_and_display_file(decrypted);
    
    free(enc_file);
    free(key_file);
}

void decrypt_without_key() {
    char* enc_file = show_file_selector(UI_TEXT_MSG_DECRYPT);
    if (!enc_file) DIE(FILE_ERROR_MSG);
    
    char* decrypted = decryption_without_key(enc_file);
    load_and_display_file(decrypted);

    free(enc_file);
}

void draw_ui() {
    draw_menu(menu_items, MENU_ITEMS, highlight);
    if (app_content) {
        draw_content(curr_filename, app_content, content_offset);         
    } else {
        draw_content("", "", 0);   
    }
}

void menu_handle() {
    switch (highlight) {
        case 0: encrypt_file(); return;
        case 1: decrypt_with_key(); return;
        case 2: decrypt_without_key(); return;
    }
}

int main() {
    init_ui();
    draw_ui();

    int ch;
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP:
                highlight = (highlight - 1 + MENU_ITEMS) % MENU_ITEMS;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % MENU_ITEMS;
                break;
            case 'j':
                content_offset++;
                break;
            case 'k':
                content_offset--;
                break;
            case 10:
                menu_handle();
                break;
        }
        draw_ui();
    }
    
    close_ui();
    free(app_content);
    free(curr_filename);

    return 0;
}