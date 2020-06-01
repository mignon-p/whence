#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __APPLE__
#include <sys/xattr.h>
#else
#include <sys/types.h>
#include <attr/xattr.h>
#endif

#include "whence.h"

static ssize_t call_getxattr (const char *path,
                              const char *name,
                              char *value,
                              size_t size) {
    return getxattr (path, name, value, size
#ifdef __APPLE__
                     , 0, 0
#endif
                     );
}

static ErrorCode errnum2ec (int errnum) {
    switch (errnum) {
    case ENOATTR:
    case ENOTSUP:
        return EC_NOATTR;
    case EISDIR:
    case ENOTDIR:
    case ENAMETOOLONG:
    case EACCES:
    case ELOOP:
    case ENOENT:
        return EC_NOFILE;
    default:
        return EC_OTHER;
    }
}

ErrorCode getAttribute (const char *fname,
                        const char *attr,
                        char **result) {
    *result = NULL;

    const ssize_t ret1 = call_getxattr (fname, attr, NULL, 0);
    ssize_t ret2 = ret1;

    if (ret1 >= 0) {
        *result = malloc (ret1);
        if (*result == NULL) {
            return EC_MEM;
        }
        ret2 = call_getxattr (fname, attr, *result, ret1);
    }

    if (ret2 < 0) {
        const int errnum = errno;
        const char *errmsg = strerror (errnum);
        free (*result);
        *result = strdup (errmsg);
        return (*result ? errnum2ec (errnum) : EC_MEM);
    }

    if (ret1 != ret2) {
        free (*result);
        *result = strdup ("attribute size mismatch");
        return (*result ? EC_OTHER : EC_MEM);
    }

    return EC_OK;
}
