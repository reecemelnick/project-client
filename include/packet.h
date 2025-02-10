#include "messages.h"

#define BUFFER_SIZE 1024

void construct_message(struct Message *header, uint8_t type, uint8_t version, uint16_t id, uint16_t length);
void serialize_message(const struct Message *header, uint8_t username, uint8_t password);
void send_and_serialize_ACC_Create_Login(int serverfd, const struct ACC_Create_Login *packet);
void send_packet(int serverfd, const uint8_t *buffer, size_t size);

void construct_connection_message(struct ConnectionMessage *connection_message, int message_type, int version);
void serialize_and_send_connection_message(int serverfd, const struct ConnectionMessage *connection_message, int *err);

uint8_t *read_entire_stream(int serverfd, int *err);

void parse_response_acc_create(const uint8_t *byte_stream, int *err);

uint16_t *parse_response_header(const uint8_t *byte_stream);

uint16_t extract_next_twobytes(const uint8_t *byte_stream, size_t *position);
