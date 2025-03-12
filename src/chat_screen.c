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
#define PACKETLEN 777
#define POLL_TIMEOUT 500
// #define TIMESTAMP_SIZE 15

static pthread_mutex_t *get_ncurses_mutex(void);
void                    make_chat_input_box(WINDOW **win, char *username, uint16_t user_id, int *cursor_pos);

struct thread_args
{
    int fd;
};

// returns a mutex to ensure only one thread modifes the ncures envrionment at once
// without this it spits undefined stuff everywhere
static pthread_mutex_t *get_ncurses_mutex(void)
{
    static pthread_mutex_t ncurses_mutex = PTHREAD_MUTEX_INITIALIZER;
    return &ncurses_mutex;
}

// thread funciton that polls server for messages
void *chat_log_thread(void *arg)
{
    int                       status;
    struct pollfd             fds;                                 // pollfd structure that will contain server fd
    int                       print_line;                          // stores the current line to print message to
    struct CHT_Send          *incoming_chat;                       // chat struct that will store values of values of chat broadcast
    const struct thread_args *args = (struct thread_args *)arg;    // structure holding thread parameters

    // make new ncurses window and inialize it with chat_log_box
    WINDOW *chat_log_win = NULL;
    pthread_mutex_lock(get_ncurses_mutex());
    chat_log_box(&chat_log_win);
    pthread_mutex_unlock(get_ncurses_mutex());

    fds.fd     = args->fd;
    fds.events = POLLIN;

    print_line = 1;

    // continous loop reading broadcast messages from server
    while(!terminate)
    {
        int ret = poll(&fds, 1, POLL_TIMEOUT);    // poll with timeout of 500ms

        // if poll fails exit error
        if(ret < 0 && errno != EINTR)
        {
            perror("poll");
            break;
        }

        // if socket has incoming data
        if(fds.revents & POLLIN)
        {
            ssize_t read_bytes;    // number of bytes read
            uint8_t read_buffer[PACKETLEN];

            read_bytes = read(args->fd, read_buffer, PACKETLEN);
            // if data is read
            if(read_bytes > 0)
            {
                status = confirm_CHT_success(read_buffer);
                if(status == -1)
                {
                    // HANDLE FAIL
                }
                else
                {
                    read_buffer[read_bytes] = '\0';
                    // create chat struct from input
                    incoming_chat = read_chat_broadcast(read_buffer);

                    // update chat log in critical section
                    pthread_mutex_lock(get_ncurses_mutex());
                    mvwprintw(chat_log_win, print_line, 1, "%s: %s", incoming_chat->username, incoming_chat->content);
                    // mvwprintw(chat_log_win, print_line, 1, "hey");
                    wrefresh(chat_log_win);
                    pthread_mutex_unlock(get_ncurses_mutex());

                    // update current line printing to and clear read buffer
                    print_line++;

                    // cleanup
                    free(incoming_chat->content);
                    free(incoming_chat->timestamp);
                    free(incoming_chat->username);
                    free(incoming_chat);
                }

                memset(read_buffer, 0, sizeof(read_buffer));
            }
            else if(read_bytes == 0)
            {
                // connection closed
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
    // endwin();
    return NULL;
}

// initializes the chat screen where messages are send and recieved
int start_chat_screen(const uint16_t user_id, uint8_t *username, int sockfd)
{
    WINDOW            *usersWin       = NULL;    // window for the users list
    WINDOW            *chat_input_win = NULL;    // window for typing chat messages
    pthread_t          chat_box_thread;          // thread that handles incoming broadcasts
    struct thread_args args;                     // struct for thread parameters
    int                flags;                    // flags of server file descriptor

    // initalize ncurses envronment
    pthread_mutex_lock(get_ncurses_mutex());
    initscr();
    cbreak();
    noecho();
    curs_set(1);
    pthread_mutex_unlock(get_ncurses_mutex());

    // set server socket to non-blocking
    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // initialize users_box
    users_box(usersWin, username);

    args.fd = sockfd;
    if(pthread_create(&chat_box_thread, NULL, chat_log_thread, &args) != 0)
    {
        perror("pthread_create failed");
        return EXIT_FAILURE;
    }
    pthread_detach(chat_box_thread);
    // read and send chat messages typed by user
    chat_input(chat_input_win, user_id, username, sockfd);
    pthread_mutex_lock(get_ncurses_mutex());
    delwin(usersWin);
    delwin(chat_input_win);
    endwin();
    pthread_mutex_unlock(get_ncurses_mutex());
    return 0;
}

// populate CHT_Send struct
void build_chat_struct(struct CHT_Send *new_chat, struct Message *chat_header, const uint8_t *message, const uint8_t *username, uint16_t id)
{
    uint8_t timestamp_buf[PACKETLEN];
    uint8_t content_buf[PACKETLEN];
    uint8_t username_buf[PACKETLEN];
    size_t  payload_len  = 0;
    size_t  message_len  = strlen((const char *)message);
    size_t  username_len = strlen((const char *)username);

    // HARDCODED: need to change
    uint8_t timestamp[]   = {0x32, 0x30, 0x32, 0x35, 0x30, 0x33, 0x30, 0x34, 0x30, 0x33, 0x30, 0x39, 0x30, 0x36, 0x5a};    // NOLINT
    size_t  timestamp_len = sizeof(timestamp);

    // size_t  timestamp_len             = TIMESTAMP_SIZE;
    // uint8_t timestamp[TIMESTAMP_SIZE] = {0};
    // get_generalized_time(&timestamp, TIMESTAMP_SIZE);

    // populate packet header
    chat_header->packet_type = CHT_Send;
    chat_header->sender_id   = id;
    chat_header->version     = VERSION;

    // set CHT_Send header
    new_chat->message = chat_header;

    // allocate space for all payload fields
    new_chat->timestamp = (uint8_t *)malloc(timestamp_len + 2);
    new_chat->content   = (uint8_t *)malloc(message_len + 2);
    new_chat->username  = (uint8_t *)malloc(username_len + 2);

    if(new_chat->timestamp == NULL || new_chat->content == NULL || new_chat->username == NULL)
    {
        perror("Failed to allocate memory");
        free(new_chat->timestamp);
        free(new_chat->content);
        free(new_chat->username);
        return;
    }

    // populate encoded timestamp
    timestamp_buf[0] = GENTIME;
    timestamp_buf[1] = (uint8_t)timestamp_len;
    memcpy(timestamp_buf + 2, timestamp, timestamp_len);
    memcpy(new_chat->timestamp, timestamp_buf, timestamp_len + 2);

    // populate encoded content
    content_buf[0] = UTF8STRING;
    content_buf[1] = (uint8_t)strlen((const char *)message);
    memcpy(content_buf + 2, message, strlen((const char *)message));
    memcpy(new_chat->content, content_buf, strlen((const char *)message) + 2);

    // populate encoded username
    username_buf[0] = UTF8STRING;
    username_buf[1] = (uint8_t)username_len;
    memcpy(username_buf + 2, username, username_len);
    memcpy(new_chat->username, username_buf, username_len + 2);

    // set payload length
    payload_len += (message_len + 2) + (timestamp_len + 2) + (strlen((const char *)username) + 2);
    chat_header->payload_len = (uint8_t)payload_len;
}

void chat_input(WINDOW *win, const uint16_t user_id, uint8_t *username, int sockfd)
{
    char message_text[INPUT_BUFFER_SIZE] = {0};
    int  i;
    // int             inputting_info;
    int             cursor_pos;
    struct CHT_Send new_chat    = {0};
    struct Message  chat_header = {0};

    // inputting_info = 1;

    // intialize chat input box
    pthread_mutex_lock(get_ncurses_mutex());
    make_chat_input_box(&win, (char *)username, user_id, &cursor_pos);
    pthread_mutex_unlock(get_ncurses_mutex());

    // continuously read chat input from user
    i = 0;
    // nodelay(win, TRUE);
    while(!terminate)
    {
        int ch = wgetch(win);    // Get input

        if(terminate)    // Exit immediately if terminate is set
        {
            printf("Exiting chat input\n");
            break;
        }

        // if character is not backspace, enter key or CTRL-C
        if((ch != 127 && ch != KEY_BACKSPACE && ch != '\n' && ch != 3) && (size_t)i < sizeof(message_text) - 1)    // NOLINT
        {
            // build message buffer                                                                     // NOLINT
            message_text[i++] = (char)ch;

            pthread_mutex_lock(get_ncurses_mutex());
            waddch(win, (chtype)ch);
            wrefresh(win);
            pthread_mutex_unlock(get_ncurses_mutex());
        }
        // if backspace delete last character from screen
        else if(ch == 127 || ch == KEY_BACKSPACE)    // NOLINT
        {
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
        // on enter pressed
        else if(ch == '\n')
        {
            message_text[i] = '\0';

            // populate chat structs and send
            build_chat_struct(&new_chat, &chat_header, (uint8_t *)message_text, username, user_id);
            send_user_message(sockfd, &new_chat);

            memset(&new_chat, 0, sizeof(new_chat));
            memset(message_text, 0, sizeof(message_text));

            // reset inputted text on GUI
            wmove(win, 2, cursor_pos);
            wclrtoeol(win);
            wrefresh(win);
            i = 0;
        }
        else if(ch == 3)
        {
            terminate = 1;
            break;
        }
    }
    wrefresh(win);
}

// confirm that a broadcast was received
int confirm_CHT_success(const uint8_t *byte_stream)
{
    if(byte_stream[0] != CHT_Send)
    {
        return -1;
    }

    return 1;
}

// make users box
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

// make chat input box
void make_chat_input_box(WINDOW **win, char *username, uint16_t user_id, int *cursor_pos)
{
    int height;
    int width;
    int starty;
    int startx;

    height = 5;      // NOLINT
    width  = 111;    // NOLINT
    starty = 31;     // NOLINT
    startx = 1;      // NOLINT

    *win = newwin(height, width, starty, startx);

    draw_box(*win);
    wmove(*win, 1, 1);
    mvwprintw(*win, 2, 2, "%s ", username);             // NOLINT
    wprintw(*win, "%d", (int)user_id);                  // NOLINT
    wprintw(*win, ":");                                 // NOLINT
    *cursor_pos = (int)strlen((char *)username) + 6;    // NOLINT
    wmove(*win, 2, *cursor_pos);

    wrefresh(*win);
}

// make chat log box
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

void get_generalized_time(uint8_t *buffer, size_t size)
{
    time_t    raw_time;
    struct tm time_info;

    time(&raw_time);
    gmtime_r(&raw_time, &time_info);

    strftime((char *)buffer, size, "%Y%m%d%H%M%SZ", &time_info);
}
