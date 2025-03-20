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

#define INPUT_BUFFER_SIZE 70
#define PACKETLEN 777
#define TIMEOUT 1000
#define WINDOW_HEIGHT 27

// #define CHT_RECV_LEN 9

// #define TIMESTAMP_SIZE 15

struct thread_args
{
    int fd;
};

static pthread_mutex_t *get_ncurses_mutex(void);

// add a node representing a message to linked list
Node *add_message_to_LL(const uint8_t *message, const uint8_t *username)
{
    size_t message_len;
    size_t username_len;
    size_t total_len;
    Node  *new_node;

    if(!message || !username)
    {
        return NULL;
    }

    message_len  = strlen((const char *)message);
    username_len = strlen((const char *)username);
    total_len    = message_len + 1 + username_len + 1;    // username:message\0

    new_node = allocate_node_data(total_len);
    if(!new_node)
    {
        return NULL;
    }

    build_node_data(new_node, message, username, message_len, username_len);

    new_node->next = NULL;

    return new_node;
}

// allocate space for node structure and node data
Node *allocate_node_data(size_t total_len)
{
    Node *new_node = (Node *)malloc(sizeof(Node));
    if(!new_node)
    {
        return NULL;
    }

    new_node->data = (char *)malloc(total_len);
    if(!new_node->data)
    {
        free(new_node);
        return NULL;
    }

    return new_node;
}

// copy and format username and message into node data
void build_node_data(Node *new_node, const uint8_t *message, const uint8_t *username, size_t message_len, size_t username_len)
{
    size_t pos;

    pos = 0;
    memcpy(new_node->data, username, username_len);    // username
    pos += username_len;

    new_node->data[pos] = ':';    // username:
    pos += 1;

    memcpy(new_node->data + pos, message, message_len);    // username:message
    pos += message_len;

    new_node->data[pos] = '\0';    // username:message/0
}

// once chat display is full, remove the head, return new head
Node *shift_nodes(Node *head)
{
    Node *temp;

    if(!head)
    {
        return NULL;
    }

    temp = head;
    head = head->next;
    free(temp->data);
    free(temp);

    return head;
}

// clear the text window completly
void clear_text_window(WINDOW *win)
{
    if(win)
    {
        werase(win);
        wrefresh(win);
    }
}

// iterate over linked list and print all messages to screen
void show_messages(WINDOW *win, Node *head)
{
    Node *itr;
    int   print_line;
    itr = head->next;

    print_line = 1;    // line to start printing at

    while(itr != NULL)
    {
        mvwprintw(win, print_line, 0, "%s", itr->data);
        wrefresh(win);
        print_line++;
        itr = itr->next;
    }
}

// free all message nodes and node data
void free_nodes(Node *head_node)
{
    Node *cur;
    Node *next;

    cur = head_node;

    while(cur != NULL)
    {
        next = cur->next;
        free(cur->data);
        free(cur);
        cur = next;
    }
}

// make dataless head inital head node
Node *initialize_head_node(void)
{
    Node *head_node;

    head_node = (Node *)malloc(sizeof(Node));
    if(!head_node)
    {
        exit(EXIT_FAILURE);
    }

    head_node->next = NULL;
    head_node->data = NULL;

    return head_node;
}

// cleaup all fields of CHT_Send struct
void free_CHT_SEND(struct CHT_Send *chat_message)
{
    free(chat_message->content);
    free(chat_message->timestamp);
    free(chat_message->username);
    free(chat_message);
}

// returns a mutex to ensure only one thread modifes the ncures envrionment at once
// without this it spits undefined stuff everywhere
static pthread_mutex_t *get_ncurses_mutex(void)
{
    static pthread_mutex_t ncurses_mutex = PTHREAD_MUTEX_INITIALIZER;
    return &ncurses_mutex;
}

// CHAT LOG PARSE AND DISPLAY LOOP. Thread funciton that polls server for messages
_Noreturn void *chat_log_thread(void *arg)
{
    int                       status;                     // status of recived message. (check that its CHT_SEND)
    int                       current_line_of_message;    // stores the current line to print message to
    struct CHT_Send          *incoming_chat;              // chat struct that will store values of values of chat broadcast
    Node                     *cur_node;                   // latest message node
    Node                     *head_node;                  // start of linked list of messages
    struct pollfd             fds[1];
    const struct thread_args *args = (struct thread_args *)arg;    // structure holding thread parameters

    // make new ncurses window and inialize it with chat_log_box within mutex
    WINDOW *chat_log_win = NULL;
    WINDOW *inner_win    = NULL;
    pthread_mutex_lock(get_ncurses_mutex());
    chat_log_box(&chat_log_win, &inner_win);
    pthread_mutex_unlock(get_ncurses_mutex());

    current_line_of_message = 1;    // start printing at line 1

    // setup poll structure. Poll for incoming data
    fds[0].fd     = args->fd;
    fds[0].events = POLLIN;

    // start linked list to store chat messages
    head_node = initialize_head_node();
    cur_node  = head_node;

    // continous loop reading broadcast messages from server
    while(!terminate)
    {
        int poll_status = poll(fds, 1, TIMEOUT);    // Timeout for 1 second
        if(poll_status == -1)
        {
            perror("poll");
            continue;
        }

        if(poll_status == 0)
        {
            continue;
        }

        if(fds[0].revents & POLLIN)
        {
            ssize_t read_bytes;    // number of bytes read
            uint8_t read_buffer[PACKETLEN];

            read_bytes = read(args->fd, read_buffer, PACKETLEN);
            if(read_bytes > 0)
            {
                status = confirm_CHT_success(read_buffer);    // confirm CHT_SEND
                if(status == -1)
                {
                    continue;
                }

                // create chat struct from input
                incoming_chat = read_chat_broadcast(read_buffer);
                // clear input buffer
                memset(read_buffer, 0, sizeof(read_buffer));

                // MAKE NODES

                cur_node->next = add_message_to_LL(incoming_chat->content, incoming_chat->username);
                if(!cur_node->next)
                {
                    free_CHT_SEND(incoming_chat);
                    terminate = 1;
                    break;
                }

                // cleanup chat message structure
                free_CHT_SEND(incoming_chat);

                pthread_mutex_lock(get_ncurses_mutex());
                clear_text_window(inner_win);
                show_messages(inner_win, head_node);
                pthread_mutex_unlock(get_ncurses_mutex());

                cur_node = cur_node->next;

                current_line_of_message++;

                if(current_line_of_message > WINDOW_HEIGHT)    // NOLINT
                {
                    head_node = shift_nodes(head_node);
                }
            }
            else if(read_bytes == 0)
            {
                continue;
            }
            else if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("read");
                continue;
            }
        }
    }

    // free all nodes
    free_nodes(head_node);

    delwin(chat_log_win);
    delwin(inner_win);

    pthread_exit(NULL);
}

// ----- MAIN THREAD ---

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

    // spawn thread to handle displaying incoming messages
    args.fd = sockfd;
    if(pthread_create(&chat_box_thread, NULL, chat_log_thread, &args) != 0)
    {
        perror("pthread_create failed");
        return EXIT_FAILURE;
    }

    // read and send chat messages typed by user
    chat_input(chat_input_win, user_id, username, sockfd);

    // wait for thread function to return
    pthread_join(chat_box_thread, NULL);

    // delete ncurses window
    pthread_mutex_lock(get_ncurses_mutex());
    if(usersWin)
    {
        delwin(usersWin);
    }
    if(chat_input_win)
    {
        delwin(chat_input_win);
    }
    endwin();
    pthread_mutex_unlock(get_ncurses_mutex());
    return 0;
}

void chat_input(WINDOW *win, const uint16_t user_id, uint8_t *username, int sockfd)
{
    char            message_text[INPUT_BUFFER_SIZE] = {0};
    int             i;
    int             cursor_pos;
    struct CHT_Send new_chat    = {0};
    struct Message  chat_header = {0};

    // intialize chat input box
    pthread_mutex_lock(get_ncurses_mutex());
    make_chat_input_box(&win, (char *)username, user_id, &cursor_pos);
    pthread_mutex_unlock(get_ncurses_mutex());

    // continuously read chat input from user
    i = 0;
    while(!terminate)
    {
        int ch = wgetch(win);    // Get input

        if(terminate)    // Exit immediately if terminate is set
        {
            break;
        }

        // if character is not backspace, enter key or CTRL-C
        if((ch != 127 && ch != KEY_BACKSPACE && ch != '\n' && ch != 3) && (size_t)i < INPUT_BUFFER_SIZE - 1)    // NOLINT
        {
            // build message buffer
            message_text[i++] = (char)ch;

            // display input to screen
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

            // cleanup last input and reset input buffer
            cleanup_input_chat(new_chat);
            memset(message_text, 0, sizeof(message_text));

            // reset inputted text on GUI
            wmove(win, 2, cursor_pos);
            wclrtoeol(win);
            wrefresh(win);
            i = 0;
        }
        // CTRL-C
        else if(ch == 3)
        {
            terminate = 1;
            break;
        }
    }
    wrefresh(win);
}

// cleanup chat message input sent to server
void cleanup_input_chat(struct CHT_Send new_chat)
{
    free(new_chat.timestamp);
    free(new_chat.content);
    free(new_chat.username);

    memset(&new_chat, 0, sizeof(new_chat));
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
void chat_log_box(WINDOW **win, WINDOW **inner)
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

    *inner = derwin(*win, height - 2, width - 2, 1, 1);

    wrefresh(*win);
    wrefresh(*inner);
}

void get_generalized_time(uint8_t *buffer, size_t size)
{
    time_t    raw_time;
    struct tm time_info;

    time(&raw_time);
    gmtime_r(&raw_time, &time_info);

    strftime((char *)buffer, size, "%Y%m%d%H%M%SZ", &time_info);
}
