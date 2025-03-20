#include "messages.h"
#include <stdint.h>

int  login_or_create(struct Message request_header, int sockfd, int form_type, int *err);
void make_login_create_req(struct Message *header, struct ACC_Create_Login request, int form_type);
void set_packet_type(int form_type, uint8_t *type);
int  handle_login_create_res(struct Message incoming_message, const uint8_t *incoming_stream, uint8_t *username, int sockfd);
