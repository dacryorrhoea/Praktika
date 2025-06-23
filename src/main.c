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

int main() {
    init_ui();
    int highlight = 0;

    draw_menu(menu_items, MENU_ITEMS, highlight);
    draw_content("", "", 0);

    int ch;
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP:
                if (--highlight < 0) highlight = MENU_ITEMS - 1;
                break;
            case KEY_DOWN:
                if (++highlight >= MENU_ITEMS) highlight = 0;
                break;
            case 'j':
                content_offset++;
                break;
            case 'k':
                if (content_offset > 0) content_offset--;
                break;
            case 10:
                switch (highlight) {
                    case 0: encrypt_file(); break;
                    case 1: decrypt_with_key(); break;
                    case 2: decrypt_without_key(); break;
                }
                break;
            case KEY_RESIZE:
                resize_ui();
                break;
        }
        draw_menu(menu_items, MENU_ITEMS, highlight);
        if (app_content) {
            draw_content(curr_filename, app_content, content_offset);         
        } else {
            draw_content("", "", 0);   
        }
        
    }
    
    close_ui();
    free(app_content);
    free(curr_filename);
    return 0;
}