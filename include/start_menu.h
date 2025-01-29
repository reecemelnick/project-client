#include <ncurses.h>

void print_menu_options(WINDOW *menuWin, const int *highlight, const char **choices);

void browse_options(const int *choice, int *highlight);

int display_menu(void);
