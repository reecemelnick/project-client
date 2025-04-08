#include "messages.h"
#include <stdint.h>

int login_or_create(struct Message request_header, int sockfd, int form_type, int *err);
