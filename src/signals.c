#include "../include/signals.h"

void setup_signal(void (*handler)(int), const int signal_type, int *err)
{
    if(signal(signal_type, handler) == SIG_ERR)
    {
        perror("Error setting up signal handler");
        *err = errno;
    }
}

void sigint_handler(int signal)
{
    if(signal == SIGINT)
    {
        terminate = 1;
    }
}
