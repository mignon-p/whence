#include "whence.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void oom (const char *file, long line) {
    const char *slash = strrchr (file, '/');
    if (slash) {
        file = slash + 1;       /* just the basename */
    }

    fprintf (stderr, CMD_NAME ": out of memory at %s:%ld\n", file, line);
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
