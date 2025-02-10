#include "../include/payload.h"
#include "../include/messages.h"

#define HEADER_SIZE 6
#define ID_INDEX 2

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

void convert_username_password(struct ACC_Create_Login *acc_create_login, const char *username, const char *password, int *err)
{
    // Converts username and password to byte stream and stores it accordingly to acc_create_login
    acc_create_login->username = string_to_bytes(username, err);

    // printf("Username (hex): ");
    // for(size_t xx = 0; xx < strlen(username) + 2; xx++)
    // {
    //     printf("%02X ", acc_create_login->username[xx]);    // %02X for hex with leading zero
    // }
    // printf("\n");

    acc_create_login->password = string_to_bytes(password, err);

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

/*
    possible responses in an incoming payload (as of milestone 1)
        0B ACC_Login_Success
        01 SYS_ERROR
        00 SYS_SUCCESS

        02 BER -> int
*/
void parse_payload(const uint8_t *byte_stream, uint16_t payload_len)
{
    size_t position;
    uint8_t tag;

    position = HEADER_SIZE + 2;
    tag = byte_stream[position];

    switch (tag) {
        case 0x0B:  // ACC_Login_Success
            //handle sys_success
            break;
        
        case SYS_Error:
            //handle sys_error
            break;

        case SYS_Success:  // SYS_SUCCESS
            //handle sys_success
            break;

        case INTEGER:  // BER -> int (for receiving ID)

            break;

        default:
            printf("Unknown Response Code");
            break;
    }
}

// THIS FUNCTION SHOULD ONLY BE USED IF WE KNOW THAT WE ARE RECEIVING AN ID
int extract_payload_login_id(const uint8_t *byte_stream, uint16_t payload_len)
{
    size_t position;
    int id;

    position = HEADER_SIZE + 2;

    id = (int) byte_stream[position];
    return id;
}

// THIS FUNCTION SHOULD ONLY BE USED IF WE KNOW THAT WE ARE RECEIVING AN ERROR CODE
int return_error_code(const uint8_t *byte_stream, uint16_t payload_len)
{
    size_t position;
    int code;

    position = HEADER_SIZE + 2;
    code = (int) byte_stream[position];
    return code;
}

// THIS FUNCTION SHOULD ONLY BE USED IF WE KNOW THAT WE ARE RECEIVING AN ERROR MESSAGE
char * return_auth_login_error(const uint8_t *byte_stream, uint16_t payload_len)
{
    size_t position;
    size_t msg_size;
    char * error_msg;

    position = HEADER_SIZE + 1;

    msg_size = (size_t) byte_stream[position];

    position++;

    char *error_msg = (char *)malloc(msg_size + 1);
    if (!error_msg) {
        return NULL;
    }

    memcpy(error_msg, &byte_stream[position], msg_size);

    error_msg[msg_size] = '\0';

    return error_msg;
}
