#include "../include/account.h"

int test(int x)
{
    return x * 2;
}

uint8_t *string_to_bytes(const char *str, int *err)
{
    uint8_t *converted_str;
    size_t   str_len;
    size_t   index = 0;
    if(str == NULL)
    {
        *err = EINVAL;    // Invalid arg
        return NULL;
    }
    str_len = strlen(str);

    converted_str = (uint8_t *)malloc((str_len + 2) * sizeof(uint8_t));
    if(converted_str == NULL)
    {
        *err = errno;
        return NULL;
    }

    converted_str[index++] = UTF8STRING;
    converted_str[index++] = (uint8_t)str_len;

    for(size_t i = 0; i < str_len; i++)
    {
        converted_str[index++] = (uint8_t)str[i];    // Use index to write
    }

    return converted_str;
}
