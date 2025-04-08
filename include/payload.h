#ifndef PAYLOAD_H
#define PAYLOAD_H

#include "messages.h"

// ACC_Create helpers
void convert_username_password(struct ACC_Create_Login *acc_create, const char *username, const char *password, int *err);

void free_acc_create(struct ACC_Create_Login *acc_create);
// ==========

#endif    // PAYLOAD_H
