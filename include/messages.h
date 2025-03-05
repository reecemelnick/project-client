#ifndef MESSAGES_H
#define MESSAGES_H
#define ENCODE_BYTES 2
#define ERROR_MESSSAGE_INDEX 11
#define ERROR_CODE_ENCODED 3

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Client Connection Protocol
struct ConnectionMessage {
    // cppcheck-suppress unusedStructMember
    uint8_t message_type;
    // cppcheck-suppress unusedStructMember
    uint8_t version;
    // cppcheck-suppress unusedStructMember
    uint8_t server_online;
    // cppcheck-suppress unusedStructMember
    uint8_t *active_server_ip;
};
// end Client Connection Protocol
// ====================
// Message Protocol

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

struct ACC_Create_Login {
    // cppcheck-suppress unusedStructMember
    struct Message *message;
    // cppcheck-suppress unusedStructMember
    uint8_t *username;
    // cppcheck-suppress unusedStructMember
    uint8_t *password;
};

struct CHT_Send {
    // cppcheck-suppress unusedStructMember
    struct Message *message;
    // cppcheck-suppress unusedStructMember
    uint8_t *timestamp;
    // cppcheck-suppress unusedStructMember
    uint8_t *content;
    // cppcheck-suppress unusedStructMember
    uint8_t *username;
};

// end Message Protocol
// ====================
typedef enum {
    CLIENT_GETIP = 0,
    MAN_RETURNIP = 1
} ConnectionPacketType;

typedef enum {
    BOOLEAN = 1,
    INTEGER = 2,
    NULL_VALUE = 5,
    ENUMERATED = 10,
    UTF8STRING = 12,
    SEQUENCE = 48, // Noted in protocol document that they ALWAYS encode it as 48 (30 in hex).
    PRINTABLESTRING = 19,
    UTCTIME = 23,
} Tag;

enum packet_types {
    SYS_Success = 0,
    SYS_Error = 1,
    LOGIN_REQUEST = 10,
    LOGIN_SUCCESS = 11,
    ACCOUNT_CREATE = 13,
    CHT_Send = 20
};

#endif    // MESSAGES_H
