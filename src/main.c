#include <ncurses.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <math.h>

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


    FILE *src = fopen("./data/voyna_i_mir.txt", "rb");
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

    FILE *tmp = fopen("./data/key1.txt", "rb");
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
    if(!table) DIE("trubble with alloc for hashTable");

    table->buckets = calloc(size, sizeof(HashNode *));
    if(!table) DIE("trubble with alloc for buckets hashTable");

    table->size = size;

    return table;
}

void hash_table_insert(HashTable *table, wchar_t symb) {
    int i = get_unicode_hash(symb, table->size);
    HashNode *node = malloc(sizeof(HashNode));
    node->symbol = symb;
    node->count = 0;
    node->next = table->buckets[i];
    table->buckets[i] = node;
}

int hash_table_contains_and_increment(HashTable *table, wchar_t symb) {
    int i = get_unicode_hash(symb, table->size);
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

size_t hash_table_contains_and_return_count(HashTable *table, wchar_t symb) {
    int i = get_unicode_hash(symb, table->size);
    HashNode *curr = table->buckets[i];
    while (curr) {
        if (curr->symbol == symb) {
            return curr->count;
        }
        curr = curr->next;
    }

    return 0;
}

void free_hash_table(HashTable *table) {
    for (size_t i = 0; i < table -> size; i++) {
        if (table -> buckets[i]) {
            HashNode *curr = table -> buckets[i];
            while (curr) {
                HashNode *tmp = curr;
                curr = curr -> next;
                free(tmp);
            };
        }
    }
    free(table -> buckets);
    free(table);
}


void decryption_without_key() {
    // якобы выбрали russian в меню
    // 
    // read letter frequencies
    size_t alphabet_length = 0;
    LetterFreq *reference_frequency = (LetterFreq *)read_letters_freq_from_bin(
        "./data/frequency/russian_frequency.bin", &alphabet_length
    );


    HashTable *hash_tbl_counter = (HashTable *)create_hash_table(200);

    for (size_t i = 0; i < alphabet_length; i++) {
        hash_table_insert(hash_tbl_counter, reference_frequency[i].letter);
        hash_table_insert(hash_tbl_counter, reference_frequency[i].letter + 32);
    };


    // counting symbols in cryption text
    FILE *file = fopen("./data/shifr.txt", "rb");
    if (!file) DIE("some troubles with files");

    int curr_symb = 0;
    while ((curr_symb = fgetwc(file)) != EOF) {
        hash_table_contains_and_increment(hash_tbl_counter, curr_symb);
    }

    fclose(file);


    // calculation of pearson's square
    LetterFreq *source_frequency = (LetterFreq *)xmalloc(
        alphabet_length, sizeof(LetterFreq)
    );

    size_t full_count = 0;
    for (size_t i = 0; i < alphabet_length; i++) {
        size_t count = 0;
        count += hash_table_contains_and_return_count(
            hash_tbl_counter, reference_frequency[i].letter
        );
        count += hash_table_contains_and_return_count(
            hash_tbl_counter, reference_frequency[i].letter + 32
        );

        full_count += count;

        source_frequency[i].letter = reference_frequency[i].letter;
        source_frequency[i].frequency = (float)count;
    };

    for (size_t i = 0; i < alphabet_length; i++) {
        source_frequency[i].frequency /= full_count;
        source_frequency[i].frequency *= 100;
    };    


    // euclidean distance square
    float euclidean_distance_square = 0;
    for (size_t i = 0; i < alphabet_length; i++) {
        euclidean_distance_square += pow(
            source_frequency[i].frequency / 100 - reference_frequency[i].frequency / 100, 2
        ); 
    };

    // Percentage of similarity according to Euclidean distance.
    wprintf(L"%.2f%%", (1 - euclidean_distance_square / 2) * 100);

    for (size_t i = 0; i < 32; i++) {
        for (size_t j = 0; j < 31; j++) {
            if (source_frequency[j].frequency < source_frequency[j + 1].frequency) {
                float temp = source_frequency[j].frequency;
                source_frequency[j].frequency = source_frequency[j + 1].frequency;
                source_frequency[j + 1].frequency = temp;
                
                wchar_t temp1 = source_frequency[j].letter;
                source_frequency[j].letter = source_frequency[j + 1].letter;
                source_frequency[j + 1].letter = temp1;
            }
        }
    }

    
    // key variants
    wchar_t **key_variants = (wchar_t **)alloc_matrix(alphabet_length, 5, sizeof(wchar_t));

    // 1 column
    for (size_t j = 0; j < 5; j++) {
        key_variants[0][j] = source_frequency[j].letter;
    }

    //  n column
    for (size_t j = 0; j < 5; j++) {
        key_variants[alphabet_length - 1][j] = source_frequency[alphabet_length - 1 - j].letter;
    }

    // 2 column
    key_variants[1][0] = source_frequency[1].letter;
    int k = 0;
    for (size_t j = 1; j < 5; j++) {
        if (k == 1) k++;
        key_variants[1][j] = source_frequency[k].letter;
        k++;
    }

    // n - 1 column
    key_variants[alphabet_length - 2][0] = source_frequency[alphabet_length - 2].letter;
    k = alphabet_length - 1;
    for (size_t j = 1; j < 5; j++) {
        if (k == alphabet_length - 2) k--;
        key_variants[alphabet_length - 2][j] = source_frequency[k].letter;
        k--;
    }
    
    for (int i = 2; i < (int)alphabet_length - 2; i++) {
        int shift = 0;
        for (int j = 0; j < 5; j++) {
            key_variants[i][j] = source_frequency[i + shift].letter;
            if (shift >= 0) shift++;
            shift *= -1;
        }
    }
    

    wprintf(L"\n");
    for (size_t i = 0; i < alphabet_length; i++) {
        wprintf(L"%lc --- %.2lf", reference_frequency[i].letter, reference_frequency[i].frequency);
        wprintf(L" | %lc --- %.2lf\n", source_frequency[i].letter, source_frequency[i].frequency);
    }


    for (size_t j = 0; j < 5; j++) {
        for (size_t i = 0; i < alphabet_length; i++) {
            wprintf(L"%lc ", key_variants[i][j]);
        }
        wprintf(L"\n");
    }


    // write key to file
    // FILE *tempe = fopen("./data/key1.txt", "wb");
    // if (!tempe) DIE("some troubles with files");
    // for (size_t j = 0; j < alphabet_length; j++) {
    //     fputwc(reference_frequency[j].letter, tempe);
    // }
    // fputwc('\n', tempe);
    // for (size_t j = 0; j < alphabet_length; j++) {
    //     fputwc(source_frequency[j].letter, tempe);
    // }
    // fclose(tempe);

    // decryption_with_key(32);


    // pearson correlation
    float sum_x = 0.0, sum_y = 0.0, sum_x2 = 0.0, sum_y2 = 0.0, sum_xy = 0.0;

    for (size_t i = 0; i < alphabet_length; ++i) {
        sum_x += source_frequency[i].frequency;
        sum_y += reference_frequency[i].frequency;
        sum_x2 += pow(source_frequency[i].frequency, 2);
        sum_y2 += pow(reference_frequency[i].frequency, 2);
        sum_xy += source_frequency[i].frequency * reference_frequency[i].frequency;
    }

    float numerator = sum_xy - (sum_x * sum_y) / alphabet_length;
    float denominator = sqrt((sum_x2 - (sum_x * sum_x) / alphabet_length) *
                             (sum_y2 - (sum_y * sum_y) / alphabet_length));

    if (!denominator) DIE("err");
    wprintf(L"\n%.2f%%", numerator / denominator * 100);


    free_hash_table(hash_tbl_counter);

    free(source_frequency);
    free(reference_frequency);
    free_matrix(key_variants, 2);
}


int main() {
    setlocale(LC_ALL, "");
    srand((unsigned) time(NULL));

    // encryption(32, 0x0410);
    // decryption_with_key(32);
    decryption_without_key();
    
    // free(letters_frequency_crypto);
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
