#ifndef __STRINGS_H_
#define __STRINGS_H_

static inline int m_toupper(int c) {
    return (c >= 'a' && c <= 'z') ? (c - 0x20) : c;
}

void* m_memcpy(void *dst, const void *src, unsigned int len);
void* m_memset(void *dst, int val, unsigned int len);
int m_memcmp (const void * ptr1, const void * ptr2, unsigned int num);

/* string functions */
int m_strncasecmp(const char *s1, const char *s2, unsigned int max_len);
int m_strncmp(const char *s1, const char *s2, unsigned int max_len);
int m_strncpy(char *dst, const char *src, unsigned int max_size);
int m_strlcpy(char *s1, const char *s2, unsigned int max_size);
int m_strnlen(const char* str, unsigned int max_size);
int m_strlen(const char* str);
const char *m_strcasestr(const char *str, const char *pattern);
long long m_strtoll(const char *str, char **end, int base);

#endif // __STRINGS_H_
