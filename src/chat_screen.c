#include "chat_screen.h"
#include "../include/messages.h"
#include "gui.h"
#include "packet.h"
#include "payload.h"
#include <ctype.h>
#include <fcntl.h>
#include <ncurses.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define INPUT_BUFFER_SIZE 128
// #define PACKETLEN 777
#define SUCCESS_RES_TYPE_INDEX 8

struct thread_args
{
    int fd;
};

void                    build_chat_struct(struct Message *new_chat_header, struct chat_message *new_chat_body, uint16_t user_id, const char *message, uint8_t *username);
void                    generate_timestamp_byte_stream(uint8_t *byte_stream);
void                   *thread_function(void *arg);
static pthread_mutex_t *get_ncurses_mutex(void);

static pthread_mutex_t *get_ncurses_mutex(void)
{
    static pthread_mutex_t ncurses_mutex = PTHREAD_MUTEX_INITIALIZER;
    return &ncurses_mutex;
}

void *thread_function(void *arg)
{
    struct pollfd             fds;
    const struct thread_args *args = (struct thread_args *)arg;

    WINDOW *chat_log_win = NULL;
    pthread_mutex_lock(get_ncurses_mutex());
    chat_log_box(&chat_log_win);
    pthread_mutex_unlock(get_ncurses_mutex());
    // printf("fd: %d\n", args->fd);

    // if(chat_log_win == NULL)
    // {
    //     fprintf(stderr, "Error: Failed to create chat log window\n");
    //     return NULL;
    // }

    fds.fd     = args->fd;
    fds.events = POLLIN;

    // printf("fds: %d\n", fds.fd);

    while(1)
    {
        int ret = poll(&fds, 1, 500);    // NOLINT   // Block indefinitely until data is available

        if(ret < 0 && errno != EINTR)
        {
            perror("poll");
            break;
        }

        if(fds.revents & POLLIN)
        {
            ssize_t read_bytes;
            uint8_t read_buffer[128];    // NOLINT

            read_bytes = read(args->fd, read_buffer, sizeof(read_buffer) - 1);
            if(read_bytes > 0)
            {
                read_buffer[read_bytes] = '\0';                              // Null-terminate string
                mvwprintw(chat_log_win, 2, 2, "%s", (char *)read_buffer);    // NOLINT
                wrefresh(chat_log_win);
                printf("reading data. Bytes read: %d\n", (int)read_bytes);
            }
            else if(read_bytes == 0)
            {
                // Connection closed
                continue;
            }
            else if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("read");
                continue;
            }
        }
    }

    delwin(chat_log_win);
    endwin();
    return NULL;
}

void generate_timestamp_byte_stream(uint8_t *byte_stream)
{
    time_t    now;
    struct tm utc_time;
    char      timestamp[16];    // NOLINT

    byte_stream[0] = 0x18;    // NOLINT
    byte_stream[1] = 0x0F;    // NOLINT

    now = time(NULL);
    gmtime_r(&now, &utc_time);

    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%SZ", &utc_time);    // NOLINT

    memcpy(byte_stream + 2, timestamp, 15);    // NOLINT
}

int start_chat_screen(const uint16_t user_id, uint8_t *username, int sockfd)
{
    WINDOW            *usersWin       = NULL;
    WINDOW            *chat_input_win = NULL;
    pthread_t          chat_box_thread;
    struct thread_args args;
    int                flags;

    pthread_mutex_lock(get_ncurses_mutex());
    initscr();
    cbreak();
    noecho();
    curs_set(1);
    pthread_mutex_unlock(get_ncurses_mutex());

    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    users_box(usersWin, username);

    printf("fd in main: %d\n", sockfd);

    args.fd = sockfd;

    if(pthread_create(&chat_box_thread, NULL, thread_function, &args) != 0)
    {
        perror("pthread_create failed");
        return EXIT_FAILURE;
    }

    pthread_detach(chat_box_thread);

    chat_input(chat_input_win, (int)user_id, username, sockfd);

    pthread_mutex_lock(get_ncurses_mutex());
    delwin(usersWin);
    endwin();
    pthread_mutex_unlock(get_ncurses_mutex());

    return 0;
}

void build_chat_struct(struct Message *new_chat_header, struct chat_message *new_chat_body, uint16_t user_id, const char *message, uint8_t *username)
{
    size_t payload_len = 0;
    size_t message_len = strlen(message);
    int    err         = 0;

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

    memcpy(new_chat_body->timestamp, timestamp, timestamp_len);

    payload_len += message_len;
    payload_len += timestamp_len;
    payload_len += strlen((char *)username);

    string_to_bytes(message, &new_chat_body->chat_message, message_len, &err);

    new_chat_header->packet_type = CHT_Send;
    new_chat_header->version     = 2;    // NOLINT
    new_chat_header->sender_id   = user_id;
    new_chat_header->payload_len = (uint16_t)(payload_len + 4);    // NOLINT

    new_chat_body->username = username;
}

void chat_input(WINDOW *win, const uint16_t user_id, uint8_t *username, int sockfd)
{
    char message_text[INPUT_BUFFER_SIZE];
    // uint8_t             res_buffer[PACKETLEN];
    // ssize_t             res;
    // int                 status;
    int height;
    int width;
    int starty;
    int startx;
    int i;
    int inputting_info;
    int cursor_pos;
    // struct pollfd       fds;
    struct Message      new_chat_header = {0};
    struct chat_message new_chat_body   = {0};

    height = 5;      // NOLINT
    width  = 111;    // NOLINT
    starty = 31;     // NOLINT
    startx = 1;      // NOLINT

    inputting_info = 1;    // NOLINT
    win            = newwin(height, width, starty, startx);
    pthread_mutex_lock(get_ncurses_mutex());
    draw_box(win);
    wmove(win, 1, 1);
    mvwprintw(win, 2, 2, "%s ", username);    // NOLINT

    wprintw(win, "%d", (int)user_id);    // NOLINT

    wprintw(win, ":");    // NOLINT

    cursor_pos = (int)strlen((char *)username) + 6;    // NOLINT
    wmove(win, 2, cursor_pos);
    pthread_mutex_unlock(get_ncurses_mutex());

    i = 0;
    while(inputting_info)
    {
        int ch;

        while((ch = wgetch(win)))
        {
            if((ch != 127 && ch != KEY_BACKSPACE && ch != '\n' && ch != 3) && (size_t)i < sizeof(message_text) - 1)    // NOLINT
            {                                                                                                          // NOLINT
                message_text[i++] = (char)ch;
                pthread_mutex_lock(get_ncurses_mutex());
                waddch(win, (chtype)ch);
                wrefresh(win);
                pthread_mutex_unlock(get_ncurses_mutex());
            }
            else if(ch == 127 || ch == KEY_BACKSPACE)    // NOLINT
            {                                            // NOLINT
                if(i > 0)
                {
                    i--;
                    pthread_mutex_lock(get_ncurses_mutex());
                    wmove(win, 2, cursor_pos + i);    // NOLINT
                    waddch(win, ' ');
                    wmove(win, 2, cursor_pos + i);    // NOLINT
                    wrefresh(win);
                    pthread_mutex_unlock(get_ncurses_mutex());
                }
            }
            else if(ch == '\n')
            {    // NOLINT
                message_text[i] = '\0';

                build_chat_struct(&new_chat_header, &new_chat_body, user_id, message_text, username);
                send_user_message(sockfd, new_chat_body, new_chat_header);

                memset(&new_chat_body, 0, sizeof(new_chat_body));
                memset(&new_chat_header, 0, sizeof(new_chat_header));

                wmove(win, 2, cursor_pos);    // NOLINT
                wclrtoeol(win);
                wrefresh(win);
                // do
                // {
                //     res = read(sockfd, res_buffer, PACKETLEN);
                // } while(res < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK));

                // if(res < 0)
                // {
                //     perror("read fail");
                //     printf("errno: %d (%s)\n", errno, strerror(errno));    // NOLINT
                // }

                // status = confirm_CHT_success(res_buffer);
                // if(status == -1)
                // {
                //     perror("chat fail");
                // }

                memset(message_text, 0, sizeof(message_text));
                i = 0;
            }
            else if(ch == 3)
            {    // NOLINT
                inputting_info = 0;
                break;
            }
        }
    }

    printf("exited loop\n");

    wrefresh(win);
}

int confirm_CHT_success(const uint8_t *byte_stream)
{
    if(byte_stream[0] != SYS_Success)
    {    // NOLINT
        return -1;
    }

    if(byte_stream[SUCCESS_RES_TYPE_INDEX] != CHT_Send)
    {    // NOLINT
        return -1;
    }

    return 1;
}

void users_box(WINDOW *usersWin, uint8_t *username)
{
    int height;
    int width;
    int starty;
    int startx;

    height = 30;    // NOLINT
    width  = 30;    // NOLINT
    starty = 1;     // NOLINT
    startx = 1;     // NOLINT

    usersWin = newwin(height, width, starty, startx);
    draw_box(usersWin);

    mvwprintw(usersWin, 1, 2, "User: %s", username);    // NOLINT
    wrefresh(usersWin);

    wrefresh(usersWin);
}

void chat_log_box(WINDOW **win)
{
    int height;
    int width;
    int starty;
    int startx;

    height = 30;    // NOLINT
    width  = 80;    // NOLINT
    starty = 1;     // NOLINT
    startx = 32;    // NOLINT

    *win = newwin(height, width, starty, startx);
    draw_box(*win);

    wrefresh(*win);
}