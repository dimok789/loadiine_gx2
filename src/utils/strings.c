#include "strings.h"

void* m_memcpy(void *dst, const void *src, unsigned int len)
{
    const unsigned char *src_ptr = (const unsigned char *)src;
    unsigned char *dst_ptr = (unsigned char *)dst;

    while(len)
    {
        *dst_ptr++ = *src_ptr++;
        --len;
    }
    return dst;
}

void* m_memset(void *dst, int val, unsigned int bytes)
{
    unsigned char *dst_ptr = (unsigned char *)dst;
    unsigned int i = 0;
    while(i < bytes)
    {
        dst_ptr[i] = val;
        ++i;
    }
    return dst;
}

int m_memcmp(const void * ptr1, const void * ptr2, unsigned int num)
{
    const unsigned char *ptr1_cpy = (const unsigned char *)ptr1;
    const unsigned char *ptr2_cpy = (const unsigned char *)ptr2;

    while(num)
    {
        int diff = (int)*ptr1_cpy - (int)*ptr2_cpy;
        if(diff != 0) {
            return diff;
        }
        ptr1_cpy++;
        ptr2_cpy++;
        --num;
    }
    return 0;
}

int m_strnlen(const char* str, unsigned int max_len) {
    unsigned int i = 0;
    while (str[i] && (i < max_len)) {
        i++;
    }
    return i;
}

int m_strlen(const char* str) {
    unsigned int i = 0;
    while (str[i]) {
        i++;
    }
    return i;
}

int m_strlcpy(char *s1, const char *s2, unsigned int max_size)
{
    if(!s1 || !s2 || !max_size)
        return 0;

    unsigned int len = 0;
    while(s2[len] && (len < (max_size-1)))
    {
        s1[len] = s2[len];
        len++;
    }
    s1[len] = 0;
    return len;
}

int m_strncpy(char *dst, const char *src, unsigned int max_size)
{
    return m_strlcpy(dst, src, max_size); // this is not correct, but mostly we need a terminating zero
}


int m_strncasecmp(const char *s1, const char *s2, unsigned int max_len) {
    if(!s1 || !s2) {
        return -1;
    }

    unsigned int len = 0;
    while(*s1 && *s2 && len < max_len)
    {
        int diff = m_toupper(*s1) - m_toupper(*s2);
        if(diff != 0) {
            return diff;
        }

        s1++;
        s2++;
        len++;
    }

    if(len == max_len) {
        return 0;
    }

    int diff = m_toupper(*s1) - m_toupper(*s2);
    if(diff != 0) {
        return diff;
    }
    return 0;
}


int m_strncmp(const char *s1, const char *s2, unsigned int max_len) {
    if(!s1 || !s2) {
        return -1;
    }

    unsigned int len = 0;
    while(*s1 && *s2 && len < max_len)
    {
        int diff = *s1 - *s2;
        if(diff != 0) {
            return diff;
        }

        s1++;
        s2++;
        len++;
    }

    if(len == max_len) {
        return 0;
    }

    int diff = *s1 - *s2;
    if(diff != 0) {
        return diff;
    }
    return 0;
}


const char *m_strcasestr(const char *str, const char *pattern)
{
    if(!str || !pattern) {
        return 0;
    }

    int len = m_strnlen(pattern, 0x1000);

    while(*str)
    {
        if(m_strncasecmp(str, pattern, len) == 0) {
            return str;
        }
        str++;
    }
    return 0;
}

long long m_strtoll(const char *str, char **end, int base)
{
    long long value = 0;
    int sign = 1;

    // skip initial spaces only
    while(*str == ' ')
        str++;

    if(*str == '-') {
        sign = -1;
        str++;
    }

    while(*str)
    {
        if(base == 16 && m_toupper(*str) == 'X') {
            str++;
            continue;
        }

        if(!(*str >= '0' && *str <= '9') && !(base == 16 && m_toupper(*str) >= 'A' && m_toupper(*str) <= 'F'))
            break;

        value *= base;

        if(m_toupper(*str) >= 'A' && m_toupper(*str) <= 'F')
            value += m_toupper(*str) - 'A' + 10;
        else
            value += *str - '0';

        str++;
    }

    if(end)
        *end = (char*) str;

    return value * sign;
}
