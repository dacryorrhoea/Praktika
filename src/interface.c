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

int operation_selecting() {

}

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
    
    wresize(menu_win, height * 0.3, 0);
    wresize(content_win, height - height * 0.3, 0);
    
    mvwin(menu_win, 0, 0);
    mvwin(content_win, height * 0.3, 0);
    
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

void draw_content(char* title, const char *text, int scroll_offset) {
    werase(content_win);
    wattron(content_win, COLOR_PAIR(4));
    box(content_win, 0, 0);
    wattroff(content_win, COLOR_PAIR(4));

    int max_y, max_x;
    getmaxyx(content_win, max_y, max_x);
    int usable_width = max_x - 4;
    int max_display_lines = max_y - 3;

    wattron(content_win, COLOR_PAIR(3));
    mvwprintw(content_win, 1, 2, "%s", title);
    wattroff(content_win, COLOR_PAIR(3));

    // текст в широкосимвольную строку
    wchar_t *wbuf = NULL;
    size_t wlen = mbstowcs(NULL, text, 0);
    if (wlen == (size_t)-1) {
        mvwprintw(content_win, 2, 2, "Ошибка кодировки текста.");
        wrefresh(content_win);
        return;
    }

    wbuf = malloc((wlen + 1) * sizeof(wchar_t));
    if (!wbuf) return;
    mbstowcs(wbuf, text, wlen + 1);

    // текст на строки с учетом переноса
    wchar_t **lines = NULL;
    int line_count = 0;
    int current_width = 0;
    wchar_t *current_line = malloc((usable_width + 1) * sizeof(wchar_t));
    if (!current_line) {
        free(wbuf);
        return;
    }

    for (size_t i = 0; i <= wlen; ++i) {
        wchar_t wc = wbuf[i];
        int width = (wc == L'\0') ? 0 : wcwidth(wc);
        if (width < 0) width = 1;

        if (wc == L'\0' || wc == L'\n' || current_width + width > usable_width) {
            current_line[current_width] = L'\0';
            lines = realloc(lines, (line_count + 1) * sizeof(wchar_t*));
            lines[line_count] = current_line;
            line_count++;
            
            current_line = malloc((usable_width + 1) * sizeof(wchar_t));
            current_width = 0;
            
            if (wc == L'\0') break;
            if (wc == L'\n') continue;
        }

        current_line[current_width++] = wc;
    }
    free(current_line);
    free(wbuf);

    if (scroll_offset < 0) scroll_offset = 0;
    if (scroll_offset > line_count - max_display_lines && line_count > max_display_lines) {
        scroll_offset = line_count - max_display_lines;
    }

    int start_line = scroll_offset;
    int end_line = start_line + max_display_lines - 1; // для учёта рамки внизу, хз почему
    if (end_line > line_count) end_line = line_count;

    wattron(content_win, COLOR_PAIR(2));
    for (int i = start_line; i < end_line; ++i) {
        int y_pos = 2 + (i - start_line);
        mvwaddwstr(content_win, y_pos, 2, lines[i]);
    }
    wattroff(content_win, COLOR_PAIR(2));

    for (int i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);

    wrefresh(content_win);
}

char* show_file_selector(char* title) {
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

    if (count == 0) {
        free(files);
        return NULL;
    }

    int highlight = 0;
    int scroll_offset = 0;
    int ch;

    int h = getmaxy(menu_win) - 2;
    int max_display = h - 1;

    while (1) {
        werase(menu_win);

        wattron(menu_win, COLOR_PAIR(4));
        box(menu_win, 0, 0);
        wattroff(menu_win, COLOR_PAIR(4));

        wattron(menu_win, COLOR_PAIR(2));
        mvwprintw(menu_win, 1, 2, "%s", title);
        wattroff(menu_win, COLOR_PAIR(2));

        int start_idx = scroll_offset;
        int end_idx = scroll_offset + max_display;
        if (end_idx > count) end_idx = count;

        for (int i = start_idx; i < end_idx; ++i) {
            int line_pos = 2 + (i - scroll_offset);

            if (i == highlight) {
                wattron(menu_win, A_REVERSE | COLOR_PAIR(1));
                mvwprintw(menu_win, line_pos, 4, "%s", files[i]);
                wattroff(menu_win, A_REVERSE | COLOR_PAIR(1));
            } else {
                wattron(menu_win, COLOR_PAIR(2));
                mvwprintw(menu_win, line_pos, 4, "%s", files[i]);
                wattroff(menu_win, COLOR_PAIR(2));
            }
        }

        wrefresh(menu_win);

        ch = getch();
        switch(ch) {
            case KEY_UP:
                highlight = (highlight - 1 + count) % count;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % count;
                break;
            case 10: // Enter
                char *selected = strdup(files[highlight]);
                for (int j = 0; j < count; j++) free(files[j]);
                free(files);
                return selected;
            case 27: // Esc
                for (int j = 0; j < count; j++) free(files[j]);
                free(files);
                return NULL;
        }

        if (highlight < scroll_offset) {
            scroll_offset = highlight;
        } else if (highlight >= scroll_offset + max_display) {
            scroll_offset = highlight - max_display + 1;
        }

        if (scroll_offset > count - max_display) {
            scroll_offset = count - max_display;
        }
        if (scroll_offset < 0) {
            scroll_offset = 0;
        }
    }
}

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