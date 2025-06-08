#include <ncurses.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

typedef struct {
    wchar_t letter;
    float frequency;
} LetterFreq;

// node for store symbol
typedef struct HashNode {
    wchar_t symbol;
    size_t count;
    struct HashNode *next;
} HashNode;

// hash table
typedef struct {
    HashNode **buckets;
    size_t size;
} HashTable;

// error message macro
#define DIE(msg) do { \
    fprintf(stderr, "[%s:%d] %s: ", __FILE__, __LINE__, (msg)); \
    perror(""); \
    exit(EXIT_FAILURE); \
} while(0)

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


void *xmalloc(size_t len, size_t size) {
    void *ptr = malloc(len * size);
    if (!ptr) DIE("xmalloc: failed");
    return ptr;
}

void **alloc_matrix(size_t rows, size_t cols, size_t size) {
    void **matrix = malloc(rows * sizeof(void *));
    if (!matrix) DIE("alloc_matrix: row allocation failed");

    for (size_t i = 0; i < rows; i++) {
        matrix[i] = malloc(cols * size);
        if (!matrix[i]) {
            for (size_t j = 0; j < i; j++)
                free(matrix[j]);
            free(matrix);
            DIE("alloc_matrix: column allocation failed");
        }
    }

    return matrix;
}

void free_matrix(void *matrix, size_t rows) {
    if (!matrix) return;
    void **mtrx = (void **)matrix;
    for (size_t i = 0; i < rows; i++)
        free(mtrx[i]);
    free(mtrx);
}


void encryption(size_t alphabet_length, int alphabet_first_char) {
    //  first step -> keygen
    int **key = (int **)alloc_matrix(2, alphabet_length, sizeof(int));


    int curr_char = alphabet_first_char;
    for (size_t i = 0; i < alphabet_length; i++) {
        key[0][i] = key[1][i] = curr_char;
        curr_char++;
    }

    
    // fisher yates shuffle
    for (size_t i = alphabet_length - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        int tmp = key[1][i];
        key[1][i] = key[1][j];
        key[1][j] = tmp;
    }


    FILE *src = fopen("./data/pushkin-metel.txt", "rb");
    FILE *tmp = fopen("./data/shifr.txt", "wb");
    if (!src || !tmp) DIE("some troubles with files");

    // read character by character, encoding the matching ones
    int curr_symb;
    while ((curr_symb = fgetwc(src)) != EOF) {
        int founded = 0;

        for (size_t i = 0; i < alphabet_length; i++) {
            if (curr_symb == key[0][i]) {
                fputwc(key[1][i], tmp);
                founded = 1;
                break;
            } else if (curr_symb == key[0][i] + 32) {
                fputwc(key[1][i] + 32, tmp);
                founded = 1;
                break;
            }
        }
        
        if (!founded) fputwc(curr_symb, tmp);
    }

    fclose(src);
    fclose(tmp);


    // write key to file
    tmp = fopen("./data/key.txt", "wb");
    if (!tmp) DIE("some troubles with files");
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < alphabet_length; j++) {
            fputwc(key[i][j], tmp);
        }
        fputwc('\n', tmp);
    }
    fclose(tmp);

    free_matrix(key, 2);
}


void decryption_with_key(size_t alphabet_length) {
    // read key
    int **key = (int **)alloc_matrix(2, alphabet_length, sizeof(int));

    FILE *tmp = fopen("./data/key.txt", "rb");
    if (!tmp) DIE("some troubles with files");

    int curr_symb = 0;
    size_t i = 0, j = 0;

    while ((curr_symb = fgetwc(tmp)) != EOF) {
        if (curr_symb != '\n') {
            key[i][j] = curr_symb; // !then perform a length check!
            j++;
        } else {
            i++;
            j = 0;
        }
    }

    fclose(tmp);
    

    FILE *src = fopen("./data/shifr.txt", "rb");
    tmp = fopen("./data/temp_deshifr.txt", "wb");
    if (!src || !tmp) DIE("some troubles with files");

    curr_symb = 0;
    while ((curr_symb = fgetwc(src)) != EOF) {
        int founded = 0;

        for (size_t i = 0; i < alphabet_length; i++) {
            if (curr_symb == key[1][i]) {
                fputwc(key[0][i], tmp);
                founded = 1;
                break;
            } else if (curr_symb == key[1][i] + 32) {
                fputwc(key[0][i] + 32, tmp);
                founded = 1;
                break;
            }
        }
        
        if (!founded) fputwc(curr_symb, tmp);
    }


    fclose(src);
    fclose(tmp);

    free_matrix(key, 2);
    key = NULL;
}


void *read_letters_freq_from_bin(const char *filename, size_t *out_count) {
    if (!out_count) return NULL;

    FILE *file = fopen(filename, "rb");
    if (!file) DIE("Failed to open file for writing");

    size_t count;
    fread(&count, sizeof(size_t), 1, file);

    LetterFreq *letter_freq_arr = (LetterFreq *)xmalloc(count, sizeof(LetterFreq));
    fread(letter_freq_arr, sizeof(LetterFreq), count, file);
    
    fclose(file);

    *out_count = count;

    return letter_freq_arr;
}


int get_unicode_hash(wchar_t symb, int table_size) {
    return (symb * 2654435761) % table_size;
}

HashTable *create_hash_table(int size) {
    HashTable *table = malloc(sizeof(HashTable));
    table->buckets = calloc(size, sizeof(HashNode *));
    table->size = size;
    return table;
}

void hash_table_insert(HashTable *table, wchar_t symb) {
    int i = unicode_hash(symb, table->size);
    HashNode *node = malloc(sizeof(HashNode));
    node->symbol = symb;
    node->count = 0;
    node->next = table->buckets[i];
    table->buckets[i] = node;
}

int hash_table_contains_and_increment(HashTable *table, wchar_t symb) {
    int i = unicode_hash(symb, table->size);
    HashNode *curr = table->buckets[i];
    while (curr) {
        if (curr->symbol == symb) {
            curr->count++;
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}


int main() {
    setlocale(LC_ALL, "");
    srand((unsigned) time(NULL));

    // alphabet_length = (size_t)alphabet_length;
    // encryption(alphabet_length, alphabet_first_char);
    // decryption_with_key(alphabet_length);


    // якобы выбрали russian в меню
    // 
    // read letter frequencies
    size_t alphabet_length = 0;
    LetterFreq *reference_frequency = (LetterFreq *)read_letters_freq_from_bin(
        "./data/frequency/russian_frequency.bin", &alphabet_length
    );


    // create empty counter for symbols
    int *symbols_counter = (int *)calloc(alphabet_length, sizeof(int));
    if (!symbols_counter) DIE('symbols_counter: error calloc');

    // counting symbols in cryption text
    FILE *file = fopen("./data/shifr.txt", "rb");
    if (!file) DIE("some troubles with files");

    int curr_symb = 0;
    while ((curr_symb = fgetwc(file)) != EOF) {
        if (curr_symb < 0x0410 || curr_symb > 0x044F) continue;

        for (size_t i = 0; i < alphabet_length; i++) {
            if (
                curr_symb == reference_frequency[i].letter ||
                curr_symb == reference_frequency[i].letter + 32
            ) {
                symbols_counter[i]++;
                break;
            }
        }
    }

    fclose(file);


    // calculation of pearson's square
    int letters_count = 0;
    for (size_t i = 0; i < alphabet_length; i++) {
        letters_count += symbols_counter[1][i];
    }


    free(reference_frequency);
    // free(letters_frequency_crypto);
    free_matrix(symbols_counter, 2);
    return 0;

    // initscr();
    // cbreak();
    // noecho();
    // keypad(stdscr, TRUE);
    // curs_set(0);
    // start_color();

    // // Цветовая пара для меню и текста
    // init_pair(1, COLOR_BLACK, COLOR_CYAN);  // выбранный пункт меню
    // init_pair(2, COLOR_GREEN, COLOR_BLACK); // текст в окне содержимого

    // int height, width;
    // getmaxyx(stdscr, height, width);

    // int menu_width = width / 4;
    // int content_width = width - menu_width;

    // WINDOW *menu_win = newwin(height, menu_width, 0, 0);
    // WINDOW *content_win = newwin(height, content_width, 0, menu_width);

    // int highlight = 0;
    // int ch;

    // // Изначальная отрисовка
    // refresh();
    // draw_menu(menu_win, highlight);
    // draw_content(content_win, content[highlight]);

    // // Главный цикл
    // while ((ch = getch()) != 'q') {
    //     switch (ch) {
    //         case KEY_UP:
    //             highlight = (highlight - 1 + MENU_ITEMS) % MENU_ITEMS;
    //             break;
    //         case KEY_DOWN:
    //             highlight = (highlight + 1) % MENU_ITEMS;
    //             break;
    //     }
    //     draw_menu(menu_win, highlight);
    //     draw_content(content_win, content[highlight]);
    // }

    // endwin();
}
