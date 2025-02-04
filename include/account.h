#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "messages.h"

uint8_t *string_to_bytes(const char *str, int *err);

void convert_username_password(struct ACC_Create *acc_create, const char *username, const char *password, int *err);

void free_acc_create(struct ACC_Create *acc_create);

#endif    // ACCOUNT_H
