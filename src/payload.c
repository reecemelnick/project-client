#include "../include/payload.h"

uint8_t *string_to_bytes(const char *str, int *err)
{
    // used to store byte stream of str
    uint8_t *converted_str;
    size_t   str_len;
    size_t   index = 0;
    if(str == NULL)
    {
        *err = EINVAL;    // Invalid arg
        return NULL;
    }
    str_len = strlen(str);

    // allocate string length + 2 number of bytes to converted_str
    //      2 extra bytes are for the field type and string length
    converted_str = (uint8_t *)malloc((str_len + 2) * sizeof(uint8_t));
    if(converted_str == NULL)
    {
        *err = errno;
        return NULL;
    }

    // set first byte as BER encoded utf8string
    converted_str[index++] = UTF8STRING;
    // set second byte to string length
    converted_str[index++] = (uint8_t)str_len;

    // converts and stores each char of str into a byte to converted_str
    for(size_t i = 0; i < str_len; i++)
    {
        converted_str[index++] = (uint8_t)str[i];    // Use index to write
    }

    return converted_str;
}

void convert_username_password(struct ACC_Create_Login *acc_create, const char *username, const char *password, int *err)
{
    // Converts username and password to byte stream and stores it accordingly to acc_create
    acc_create->username = string_to_bytes(username, err);

    // printf("Username (hex): ");
    // for(size_t xx = 0; xx < strlen(username) + 2; xx++)
    // {
    //     printf("%02X ", acc_create_login->username[xx]);    // %02X for hex with leading zero
    // }
    // printf("\n");

    acc_create->password = string_to_bytes(password, err);

    // printf("Password (hex): ");
    // for(size_t yy = 0; yy < strlen(password) + 2; yy++)
    // {
    //     printf("%02X ", acc_create_login->password[yy]);    // %02X for hex with leading zero
    // }
    // printf("\n");
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
