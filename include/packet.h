#include "messages.h"

void             construct_message(struct Message *header, uint8_t type, uint8_t version, uint16_t id, uint16_t length);
void             serialize_message(const struct Message *header, uint8_t username, uint8_t password);
void             send_and_serialize_ACC_Create_Login(int serverfd, const struct ACC_Create_Login *packet);
void             send_packet(int serverfd, const uint8_t *buffer, size_t size);
void             construct_connection_message(struct ConnectionMessage *connection_message, uint8_t message_type, uint8_t version, uint8_t server_online);
void             send_and_serialize_connection_message(int server_manager_fd, const struct ConnectionMessage *connection_message);
void             send_packet_t(const uint8_t *buffer, size_t packet_size);
void             read_entire_stream(int serverfd, uint8_t **bytestream, size_t *size, int *err);
void             parse_response_header(const uint8_t *byte_stream, struct Message *incoming_message);
uint16_t         extract_next_twobytes(const uint8_t *byte_stream, size_t *position);
int              set_fd_non_blocking(int fd);
void             parse_and_extract_message(const uint8_t *byte_stream, uint8_t *message, size_t offset, size_t message_length);
uint8_t         *parse_and_extract_payload_value(const uint8_t *byte_stream, size_t payload_value_size);
uint8_t         *get_error_code(const uint8_t *byte_stream, size_t size);
void             get_user_id(const uint8_t *byte_stream, uint16_t *user_id);
struct CHT_Send *read_chat_broadcast(const uint8_t *byte_stream);
void             send_user_message(int fd, struct CHT_Send *cht_packet);
uint8_t         *construct_cht_payload(struct CHT_Send *cht_packet);
void             make_logout_req(int server_fd, uint16_t sender_id);
void             send_and_serialize_message(int server_fd, const struct Message *message);
void             parse_connection_message_header(const uint8_t *byte_stream, struct ConnectionMessage *incoming_message);
void             build_chat_struct(struct CHT_Send *new_chat, struct Message *chat_header, const uint8_t *message, const uint8_t *username, uint16_t id);
void             get_generalized_time(uint8_t *buffer, size_t size);
