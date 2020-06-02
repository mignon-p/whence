#ifndef WHENCE_H
#define WHENCE_H

#include <stddef.h>

typedef enum ErrorCode {
    EC_OK = 0,
    EC_NOATTR = 1,
    EC_NOFILE = 2,
    EC_OTHER = 3,
    EC_CMDLINE = 4,
    EC_MEM = 5
} ErrorCode;

ErrorCode getAttribute (const char *fname,
                        const char *attr,
                        char **result,
                        size_t *length);

ErrorCode printProps (const void *data, size_t length);

void checkProps (const void *data, size_t length, const char* filename);

#define CHECK_NULL(x) \
    do { if ((x) == NULL) oom (__FILE__, __LINE__); } while (0)

void oom (const char *file, long line);

typedef struct ArrayList {
    char **strings;
    size_t size;
    size_t capacity;
} ArrayList;

void AL_init (ArrayList *al);
void AL_add (ArrayList *al, const char *str);
void AL_clear (ArrayList *al);
void AL_cleanup (ArrayList *al);

ErrorCode props2list (const void *data, size_t length, ArrayList *dest);

#endif  /* WHENCE_H */
