#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include "interface.h"
#include <ncurses.h>
#include <wchar.h>
#include <locale.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <wctype.h>

static WINDOW *menu_win;
static WINDOW *content_win;

void init_ui() {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    
    init_pair(1, COLOR_BLACK, COLOR_CYAN);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_BLACK, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    
    int height, width;
    getmaxyx(stdscr, height, width);

    int menu_height = height * 0.3;
    int content_height = height - menu_height;
    
    menu_win = newwin(menu_height, width, 0, 0);
    content_win = newwin(content_height, width, menu_height, 0);

    wbkgd(menu_win, COLOR_PAIR(3));
    wbkgd(content_win, COLOR_PAIR(3));

    box(menu_win, 0, 0);
    box(content_win, 0, 0);

    refresh();
}

void close_ui() {
    delwin(menu_win);
    delwin(content_win);
    endwin();
}

void resize_ui() {
    int height, width;
    getmaxyx(stdscr, height, width);
    
    wresize(menu_win, height, width / 4);
    wresize(content_win, height, width - width / 4);
    
    mvwin(menu_win, 0, 0);
    mvwin(content_win, 0, width / 4);
    
    clear();
    refresh();
}

void draw_menu(const char *items[], int num_items, int highlight) {
    // clear win becausew werase take artefact
    int h = getmaxy(menu_win) - 2;
    int w = getmaxx(menu_win) - 2;
    
    for (int y = 1; y <= h; y++) {
        mvwhline(menu_win, y, 1, ' ', w);
    }
    
    wattron(menu_win, COLOR_PAIR(4));
    box(menu_win, 0, 0);
    wattroff(menu_win, COLOR_PAIR(4));

    for (int i = 0; i < num_items; ++i) {
        const char *item = items[i];

        int start_y = (h - num_items) / 2 + 1;

        if (i == highlight) {
            wattron(menu_win, A_REVERSE | COLOR_PAIR(1));
            mvwaddnstr(menu_win, start_y + i, 4, item, w - 1);
            wattroff(menu_win, A_REVERSE | COLOR_PAIR(1));
        } else {
            wattron(menu_win, COLOR_PAIR(2));
            mvwaddnstr(menu_win, start_y + i, 4, item, w - 1);
            wattroff(menu_win, COLOR_PAIR(2));
        }
    }
    wrefresh(menu_win);
}

void draw_content(const char *text) {
    werase(content_win);

    wattron(content_win, COLOR_PAIR(4));
    box(content_win, 0, 0);
    wattroff(content_win, COLOR_PAIR(4));

    int max_y, max_x;
    getmaxyx(content_win, max_y, max_x);
    int row = 1, col = 2;
    int usable_width = max_x - 4;

    // выделяем буфер для широких символов
    wchar_t wbuf[4096];
    size_t wlen = mbstowcs(wbuf, text, sizeof(wbuf) / sizeof(wchar_t));
    if (wlen == (size_t)-1) {
        mvwprintw(content_win, 1, 2, "Ошибка кодировки текста.");
        wrefresh(content_win);
        return;
    }

    wattron(content_win, COLOR_PAIR(2));

    for (size_t i = 0; i < wlen && row < max_y - 1; ++i) {
        wchar_t wc = wbuf[i];

        if (wc == L'\n') {
            row++;
            col = 2;
            continue;
        }

        int width = wcwidth(wc);
        if (width < 0) width = 1;

        if (col + width > max_x - 2) {
            row++;
            col = 2;
            if (row >= max_y - 1) break;
        }

        mvwaddnwstr(content_win, row, col, &wc, 1);
        col += width;
    }

    wattroff(content_win, COLOR_PAIR(2));
    wrefresh(content_win);
}

char *show_file_selector() {
    DIR *dir = opendir("data");
    if (!dir) return NULL;

    struct dirent *entry;
    char **files = NULL;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) count++;
    }

    rewinddir(dir);
    files = malloc(count * sizeof(char*));
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            files[i++] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    int highlight = 0;
    int ch;

    while (1) {
        werase(menu_win);

        wattron(menu_win, COLOR_PAIR(4));
        box(menu_win, 0, 0);
        wattroff(menu_win, COLOR_PAIR(4));

        int h = getmaxy(menu_win) - 2;
        for (int i = 0; i < count; ++i) {
            int start_y = (h - count) / 2 + 1;

            if (i == highlight) {
                wattron(menu_win, A_REVERSE | COLOR_PAIR(1));
                mvwprintw(menu_win, start_y + i, 4, "%s", files[i]);
                wattroff(menu_win, A_REVERSE | COLOR_PAIR(1));
            } else {
                wattron(menu_win, COLOR_PAIR(2));
                mvwprintw(menu_win, start_y + i, 4, "%s", files[i]);
                wattroff(menu_win, COLOR_PAIR(2));
            }
        }
        wrefresh(menu_win);

        ch = getch();
        if (ch == KEY_UP)
            highlight = (highlight - 1 + count) % count;
        else if (ch == KEY_DOWN)
            highlight = (highlight + 1) % count;
        else if (ch == 10) { // Enter
            char *selected = strdup(files[highlight]);
            for (int j = 0; j < count; j++) free(files[j]);
            free(files);
            return selected;
        } else if (ch == 27) { // Esc
            for (int j = 0; j < count; j++) free(files[j]);
            free(files);
            return NULL;
        }
    }
}