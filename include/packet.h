#include "messages.h"

void construct_message(struct Message *header, uint8_t type, uint8_t version, uint16_t id, uint16_t length);
void serialize_message(const struct Message *header, uint8_t username, uint8_t password);
