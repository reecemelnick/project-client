#include <network_utils.h>

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern int terminate;

void setup_signal(void (*handler)(int), int signal_type, int *err);

void sigint_handler(int signal);
