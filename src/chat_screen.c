
#include "chat_screen.h"
#include "../include/messages.h"
#include "gui.h"
#include "packet.h"
#include "payload.h"
#include <ctype.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define INPUT_BUFFER_SIZE 128
#define PACKETLEN 777

void build_chat_struct(struct Message *new_chat_header, struct chat_message *new_chat_body, uint16_t user_id, const char *message, uint8_t *username);

void generate_timestamp_byte_stream(uint8_t *byte_stream);

void generate_timestamp_byte_stream(uint8_t *byte_stream)
{
    // Declare variables
    time_t    now;
    struct tm utc_time;
    char      timestamp[16];    // NOLINT // 15 chars + null terminator

    // Set metadata bytes
    byte_stream[0] = 0x18;    // NOLINT
    byte_stream[1] = 0x0F;    // NOLINT

    // Get current time and convert to UTC (thread-safe)
    now = time(NULL);
    gmtime_r(&now, &utc_time);

    // Format as "YYYYMMDDhhmmssZ"
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%SZ", &utc_time);

    // Copy formatted timestamp into byte stream
    memcpy(byte_stream + 2, timestamp, 15);    // NOLINT
}

int start_chat_screen(const uint16_t user_id, uint8_t *username, int sockfd)
{
    WINDOW *usersWin       = NULL;
    WINDOW *chat_log_win   = NULL;
    WINDOW *chat_input_win = NULL;

    initscr();
    cbreak();
    noecho();
    curs_set(1);

    users_box(usersWin, username);
    chat_log_box(chat_log_win);
    chat_input(chat_input_win, (int)user_id, username, sockfd);

    delwin(usersWin);
    delwin(chat_log_win);
    endwin();

    return 0;
}

void build_chat_struct(struct Message *new_chat_header, struct chat_message *new_chat_body, uint16_t user_id, const char *message, uint8_t *username)
{
    size_t payload_len = 0;
    size_t message_len = strlen(message);
    int    err         = 0;

    // Hardcoded timestamp
    uint8_t timestamp[]   = {0x18, 0x0F, 0x32, 0x30, 0x32, 0x35, 0x30, 0x33, 0x30, 0x34, 0x30, 0x33, 0x30, 0x39, 0x30, 0x36, 0x5a};    // NOLINT
    size_t  timestamp_len = sizeof(timestamp);

    if(new_chat_body->timestamp == NULL)
    {
        new_chat_body->timestamp = (uint8_t *)malloc(timestamp_len);
        if(new_chat_body->timestamp == NULL)
        {
            perror("Failed to allocate memory for timestamp");
            return;
        }
    }

    // Set timestamp in chat body
    memcpy(new_chat_body->timestamp, timestamp, timestamp_len);

    // Calculate payload length
    payload_len += message_len;                 // Message length
    payload_len += timestamp_len;               // Timestamp length
    payload_len += strlen((char *)username);    // Username length (assuming it's null-terminated)

    // Convert the message string to bytes
    string_to_bytes(message, &new_chat_body->chat_message, message_len, &err);

    // Set header fields
    new_chat_header->packet_type = CHT_Send;
    new_chat_header->version     = 2;
    new_chat_header->sender_id   = user_id;
    new_chat_header->payload_len = (uint16_t)(payload_len + 4);    // Including header size (e.g., 4 for packet type and version)

    // Set username and timestamp in chat body
    new_chat_body->username = username;
}

void chat_input(WINDOW *win, const uint16_t user_id, uint8_t *username, int sockfd)
{
    char                message_text[INPUT_BUFFER_SIZE];
    uint8_t             res_buffer[PACKETLEN];
    int                 height;
    int                 width;
    int                 starty;
    int                 startx;
    int                 i;
    int                 inputting_info;
    int                 cursor_pos;
    struct Message      new_chat_header = {0};
    struct chat_message new_chat_body   = {0};

    height = 5;      // NOLINT
    width  = 111;    // NOLINT
    starty = 31;     // NOLINT
    startx = 1;

    inputting_info = 1;
    win            = newwin(height, width, starty, startx);
    draw_box(win);
    wmove(win, 1, 1);
    mvwprintw(win, 2, 2, "%s ", username);    // NOLINT

    wprintw(win, "%d", (int)user_id);

    wprintw(win, ":");    // NOLINT

    cursor_pos = (int)strlen((char *)username) + 6;    // NOLINT // after username + user_id + ": "
    wmove(win, 2, cursor_pos);

    // read input. When press enter build struct and clear input bar and set cursor back to start
    i = 0;
    while(inputting_info)
    {
        int ch;

        while((ch = wgetch(win)))
        {
            if((ch != 127 && ch != KEY_BACKSPACE && ch != '\n' && ch != 3) && (size_t)i < sizeof(message_text) - 1)    // NOLINT
            {
                message_text[i++] = (char)ch;
                waddch(win, (chtype)ch);
                wrefresh(win);
            }
            else if(ch == 127 || ch == KEY_BACKSPACE)    // NOLINT
            {
                if(i > 0)
                {
                    i--;
                    wmove(win, 2, cursor_pos + i);    // NOLINT    // NOLINT
                    waddch(win, ' ');
                    wmove(win, 2, cursor_pos + i);    // NOLINT                    wrefresh(win);
                }
            }
            else if(ch == '\n')    // NOLINT
            {
                message_text[i] = '\0';

                // build struct
                build_chat_struct(&new_chat_header, &new_chat_body, user_id, message_text, username);
                send_user_message(sockfd, new_chat_body, new_chat_header);

                // free(new_chat_body.timestamp);
                memset(&new_chat_body, 0, sizeof(new_chat_body));
                memset(&new_chat_header, 0, sizeof(new_chat_header));

                wmove(win, 2, cursor_pos);    // NOLINT // Move cursor to start of input field
                wclrtoeol(win);               // Clear from cursor position to end of line
                wrefresh(win);

                memset(message_text, 0, sizeof(message_text));

                read(sockfd, res_buffer, PACKETLEN);

                i = 0;
            }
            else if(ch == 3)    // NOLINT
            {
                inputting_info = 0;
                break;
            }
        }
    }

    // build message struct

    wrefresh(win);
}

void users_box(WINDOW *usersWin, uint8_t *username)
{
    int height;
    int width;
    int starty;
    int startx;

    height = 30;    // NOLINT
    width  = 30;    // NOLINT
    starty = 1;
    startx = 1;

    usersWin = newwin(height, width, starty, startx);
    draw_box(usersWin);

    mvwprintw(usersWin, 1, 2, "User: %s", username);
    wrefresh(usersWin);    // Refresh the window to show changes

    wrefresh(usersWin);
}

void chat_log_box(WINDOW *win)
{
    int height;
    int width;
    int starty;
    int startx;

    height = 30;    // NOLINT
    width  = 80;    // NOLINT
    starty = 1;
    startx = 32;    // NOLINT

    win = newwin(height, width, starty, startx);
    draw_box(win);

    wrefresh(win);
}
