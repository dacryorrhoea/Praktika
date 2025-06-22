#ifndef INTERFACE_H
#define INTERFACE_H

void init_ui();
void close_ui();
void resize_ui();
// void draw_file_list();
void draw_menu(const char *items[], int num_items, int highlight);
void draw_content(char* title, const char *text, int scroll_offset);
char* show_file_selector(char* title);
char* read_file_content(const char *filename);

#endif