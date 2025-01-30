#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

struct Message {
    // cppcheck-suppress unusedStructMember
    uint8_t packet_type;
    // cppcheck-suppress unusedStructMember
    uint8_t version;
    // cppcheck-suppress unusedStructMember
    uint16_t sender_id;
    // cppcheck-suppress unusedStructMember
    uint16_t payload_len;
};

struct ACC_Create {
    // cppcheck-suppress unusedStructMember
    struct Message *message;
    // cppcheck-suppress unusedStructMember
    uint8_t seq_len[2]; // First byte is BER 30, second is seq. length
    // cppcheck-suppress unusedStructMember
    uint8_t *username;
    // cppcheck-suppress unusedStructMember
    uint8_t *password;
};

typedef enum {
    BOOLEAN = 1,
    INTEGER = 2,
    NULL_VALUE = 5,
    ENUMERATED = 10,
    UTF8STRING = 12,
    SEQUENCE = 16,
    PRINTABLESTRING = 19,
    UTCTIME = 23
} Tag;

#endif    // MESSAGES_H
