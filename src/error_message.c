#include "error_message.h"
#include "gui.h"
#include <ctype.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HEIGHT 10
#define WIDTH 40

void display_error_message(const uint8_t *byte_stream, const uint8_t *err_code, size_t message_size, size_t code_size)
{
    int      height;
    int      width;
    int      starty;
    int      startx;
    WINDOW  *win;
    char    *error_message;
    uint8_t *error_code;

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    height = HEIGHT;
    width  = WIDTH;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;

    error_message = (char *)malloc(message_size * sizeof(char));
    if(!error_message)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < message_size; i++)
    {
        error_message[i] = (char)byte_stream[i];
    }

    error_code = (uint8_t *)malloc(code_size * sizeof(uint8_t));
    if(!error_code)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < code_size; i++)
    {
        error_code[i] = err_code[i];
    }

    // Create a new window for the GUI
    win = newwin(height, width, starty, startx);
    draw_box(win);

    mvwprintw(win, 3, 2, "%s", error_message);    // NOLINT
    mvwprintw(win, 4, 2, "Error code:");          // NOLINT
    for(size_t i = 0; i < code_size; i++)
    {
        wprintw(win, " %02X", error_code[i]);
    }

    mvwprintw(win, 1, 2, "Failed to log in");

    mvwprintw(win, 8, 2, "Press any key to exit...");    // NOLINT
    wrefresh(win);

    wgetch(win);

    delwin(win);
    endwin();

    free(error_message);
    free(error_code);
}