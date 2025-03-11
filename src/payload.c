#include "../include/payload.h"

void string_to_bytes(const char *str, uint8_t **message_str, size_t len, int *err)
{
    // used to store byte stream of str
    if(str == NULL)
    {
        *err = EINVAL;    // Invalid arg
        return;
    }

    *message_str = (uint8_t *)malloc((len + 1) * sizeof(uint8_t));
    if(!*message_str)
    {
        perror("malloc");
        return;
    }

    memcpy(*message_str, str, len);
    (*message_str)[len] = '\0';
}

void convert_username_password(struct ACC_Create_Login *acc_create, const char *username, const char *password, int *err)
{
    // Converts username and password to byte stream and stores it accordingly to acc_create
    size_t len = strlen(username);
    printf("username length: %zu\n", len);
    string_to_bytes(username, &acc_create->username, len, err);

    len = strlen(password);
    printf("password length: %zu\n", len);
    string_to_bytes(username, &acc_create->password, len, err);
}

void free_acc_create(struct ACC_Create_Login *acc_create_login)
{
    if(acc_create_login->username != NULL)
    {
        free(acc_create_login->username);
    }
    if(acc_create_login->password != NULL)
    {
        free(acc_create_login->password);
    }
}
