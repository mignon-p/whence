#include "whence.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *my_basename (const char *file) {
    const char *slash = strrchr (file, '/');
    if (slash) {
        return slash + 1;
    } else {
        return file;
    }
}

void oom (const char *file, long line) {
    fprintf (stderr, CMD_NAME ": out of memory at %s:%ld\n",
             my_basename (file), line);
    exit (EC_MEM);
}

ErrorCode combineErrors (ErrorCode ec1, ErrorCode ec2) {
    if (ec2 > ec1 && ec2 > EC_NOATTR) {
        return ec2;
    } else if (ec1 > EC_NOATTR) {
        return ec1;
    } else if (ec2 == EC_OK) {
        return EC_OK;
    } else {
        return ec1;
    }
}

char *my_strdup (const char *s, const char *file, long line) {
    if (s == NULL) {
        fprintf (stderr, CMD_NAME ": strdup called on NULL at %s:%ld\n",
                 my_basename (file), line);
        exit (EC_OTHER);
    }

    char *ret = strdup (s);
    if (ret == NULL) {
        oom (file, line);
    }

    return ret;
}
