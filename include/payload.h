#ifndef PAYLOAD_H
#define PAYLOAD_H

#include "messages.h"

// Helper functions
uint8_t *string_to_bytes(const char *str, int *err);
// ==========

// ACC_Create helpers
void convert_username_password(struct ACC_Create *acc_create, const char *username, const char *password, int *err);

void free_acc_create(struct ACC_Create *acc_create);
// ==========

#endif    // PAYLOAD_H
