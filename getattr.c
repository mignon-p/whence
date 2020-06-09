/*
 * Copyright (c) 2020 Patrick Pelletier
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "whence.h"

#ifndef _WIN32

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __APPLE__
#include <sys/xattr.h>
#elif defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/extattr.h>
#elif defined(__linux__)
#include <sys/types.h>
#include <sys/xattr.h>
// #include <attr/xattr.h>
#else
#error "Unknown operating system"
#endif

static ssize_t call_getxattr (const char *path,
                              const char *name,
                              char *value,
                              size_t size) {
#ifdef __APPLE__
    return getxattr (path, name, value, size, 0, 0);
#elif defined(__FreeBSD__)
    return extattr_get_file (path, EXTATTR_NAMESPACE_USER, name, value, size);
#elif defined (__linux__)
    return getxattr (path, name, value, size);
#endif
}

static ErrorCode errnum2ec (int errnum) {
    switch (errnum) {
#ifdef ENOATTR
    case ENOATTR:
#elif defined (ENODATA)
    case ENODATA:
#endif
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
                        char **result,
                        size_t *length) {
    *result = NULL;
    *length = 0;

    const ssize_t ret1 = call_getxattr (fname, attr, NULL, 0);
    ssize_t ret2 = ret1;

    if (ret1 >= 0) {
        *result = malloc (ret1 + 1); /* leave room for NUL terminator */
        CHECK_NULL (*result);
        ret2 = call_getxattr (fname, attr, *result, ret1 + 1);
    }

    if (ret2 < 0) {
        const int errnum = errno;
        const char *errmsg = strerror (errnum);
        free (*result);
        *result = MY_STRDUP (errmsg);
        *length = strlen (*result);
        return errnum2ec (errnum);
    }

    if (ret1 != ret2) {
        free (*result);
        *result = MY_STRDUP ("attribute size mismatch");
        *length = strlen (*result);
        return EC_OTHER;
    }

    /* NUL terminate (not included in length) to make
     * working with strings easier. */
    (*result)[ret2] = 0;

    *length = ret2;
    return EC_OK;
}

/* This function only does anything on Windows (see windows.c).
 * On UNIX, all we have to do is copy the filename, so it can
 * be freed later.
 */
char *fixFilename (const char *fname, int32_t *drives) {
    return MY_STRDUP (fname);
}

#endif  /* not _WIN32 */
