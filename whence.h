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

#endif  /* WHENCE_H */
