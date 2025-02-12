#include "messages.h"

void construct_message(struct Message *header, uint8_t type, uint8_t version, uint16_t id, uint16_t length);
void serialize_message(const struct Message *header, uint8_t username, uint8_t password);
void send_and_serialize_ACC_Create_Login(int serverfd, const struct ACC_Create_Login *packet);
void send_packet(int serverfd, const uint8_t *buffer, size_t size);

void construct_connection_message(struct ConnectionMessage *connection_message, int message_type, int version);
void serialize_and_send_connection_message(int serverfd, const struct ConnectionMessage *connection_message, int *err);

void send_packet_t(const uint8_t *buffer, size_t packet_size);

uint8_t *read_entire_stream(int serverfd, size_t *size, int *err);

void parse_response_header(const uint8_t *byte_stream, struct Message *incoming_message);

uint16_t extract_next_twobytes(const uint8_t *byte_stream, size_t *position);

int set_fd_non_blocking(int fd);

uint8_t *parse_and_extract_message(const uint8_t *byte_stream, size_t offset, size_t message_length, int *err);

uint8_t *parse_and_extract_payload_value(const uint8_t *byte_stream, size_t payload_value_size);

uint8_t *get_error_code(const uint8_t *byte_stream, size_t size);

uint16_t get_user_id(const uint8_t *byte_stream);
