#ifndef MESSAGES_H
#define MESSAGES_H

struct Message {
    uint8_t packet_type;
    uint8_t version;
    uint16_t sender_id;
    uint16_t payload_len;
};

struct ACC_Create {
    struct Message *message;
    uint8_t seq_len[2]; // First byte is BER 30, second is seq. length
    uint8_t *username;
    uint8_t *password;
};

#endif    // MESSAGES_H
