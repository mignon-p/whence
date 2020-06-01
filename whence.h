#ifndef WHENCE_H
#define WHENCE_H

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

#endif  /* WHENCE_H */
