#include "gui.h"
#include <ncurses.h>

void draw_box(WINDOW *win)
{
    box(win, 0, 0);
    wrefresh(win);
}
