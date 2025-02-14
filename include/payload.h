#ifndef PAYLOAD_H
#define PAYLOAD_H

#include "messages.h"

// Helper functions
void string_to_bytes(const char *str, uint8_t **message_str, size_t len, int *err);
// ==========

// ACC_Create helpers
void convert_username_password(struct ACC_Create_Login *acc_create, const char *username, const char *password, int *err);

void free_acc_create(struct ACC_Create_Login *acc_create);
// ==========

#endif    // PAYLOAD_H
