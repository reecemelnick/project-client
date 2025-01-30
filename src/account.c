#include "../include/account.h"

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
