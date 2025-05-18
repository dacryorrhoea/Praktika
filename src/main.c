#include <ncurses.h>
#include <locale.h>
#include <string.h>

// Количество пунктов меню
#define MENU_ITEMS 3

const char *menu_items[MENU_ITEMS] = {
    "Первый",
    "Второй",
    "Третий"
};

const char *content[MENU_ITEMS] = {
    "Содержимое для первого пункта",
    "Содержимое для второго пункта",
    "Содержимое для третьего пункта"
};

void draw_menu(WINDOW *win, int highlight) {
    box(win, 0, 0);
    for (int i = 0; i < MENU_ITEMS; ++i) {
        if (i == highlight) {
            wattron(win, A_REVERSE | COLOR_PAIR(1));
            mvwprintw(win, i + 1, 2, "%s", menu_items[i]);
            wattroff(win, A_REVERSE | COLOR_PAIR(1));
        } else {
            mvwprintw(win, i + 1, 2, "%s", menu_items[i]);
        }
    }
    wrefresh(win);
}

void draw_content(WINDOW *win, const char *text) {
    werase(win);
    box(win, 0, 0);
    wattron(win, COLOR_PAIR(2));
    mvwprintw(win, 1, 2, "%s", text);
    wattroff(win, COLOR_PAIR(2));
    wrefresh(win);
}

int main() {
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();

    // Цветовая пара для меню и текста
    init_pair(1, COLOR_BLACK, COLOR_CYAN);  // выбранный пункт меню
    init_pair(2, COLOR_GREEN, COLOR_BLACK); // текст в окне содержимого

    int height, width;
    getmaxyx(stdscr, height, width);

    int menu_width = width / 4;
    int content_width = width - menu_width;

    WINDOW *menu_win = newwin(height, menu_width, 0, 0);
    WINDOW *content_win = newwin(height, content_width, 0, menu_width);

    int highlight = 0;
    int ch;

    // Изначальная отрисовка
    refresh();
    draw_menu(menu_win, highlight);
    draw_content(content_win, content[highlight]);

    // Главный цикл
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP:
                highlight = (highlight - 1 + MENU_ITEMS) % MENU_ITEMS;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % MENU_ITEMS;
                break;
        }
        draw_menu(menu_win, highlight);
        draw_content(content_win, content[highlight]);
    }

    endwin();
    return 0;
}
