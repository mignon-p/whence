#include "whence.h"

#ifdef __APPLE__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static ErrorCode parse_quarantine (Attributes *dest, const char *s) {
    ArrayList al;

    AL_init (&al);
    split (s, ";", &al);

    if (al.size != 4) {
        if (dest->error == NULL) {
            char buf[80];
            snprintf (buf, sizeof (buf),
                      "Expected 4 fields in com.apple.quarantine, but got %lu",
                      (unsigned long) al.size);
            dest->error = strdup (buf);
            CHECK_NULL (dest->error);
        }

        AL_cleanup (&al);
        return EC_OTHER;
    }

    const char *hexdate = al.strings[1];
    const char *application = al.strings[2];
    const char *uuid = al.strings[3];

    char *endptr = NULL;
    errno = 0;
    const unsigned long long date = strtoull (hexdate, &endptr, 16);
    const int errnum = errno;
    if (errnum != 0 || *hexdate == 0 || *endptr != 0) {
        char buf[80];
        if (dest->error == NULL) {
            if (errnum != 0) {
                dest->error = strdup (strerror (errnum));
            } else {
                snprintf (buf, sizeof (buf),
                          "'%s' is not a valid hex number.", hexdate);
                dest->error = buf;
            }

            CHECK_NULL (dest->error);
        }

        AL_cleanup (&al);
        return EC_OTHER;
    } else {
        dest->date = (time_t) date;
    }

    if (dest->application == NULL) {
        dest->application = strdup (application);
        CHECK_NULL (dest->application);
    }

    ErrorCode ret = EC_OK;
    if (*uuid) {
        // ret = lookup_uuid (dest, uuid);
    }

    AL_cleanup (&al);
    return ret;
}

static ErrorCode parse_wherefroms (Attributes *dest,
                                   const char *attr,
                                   size_t attrLen) {
    ArrayList al;

    AL_init (&al);
    const ErrorCode ec = props2list (attr, attrLen, &al);
    if (ec != EC_OK) {
        if (dest->error == NULL) {
            dest->error =
                strdup (al.size == 1 ? al.strings[0] : "Unknown error");
            CHECK_NULL (dest->error);
        }

        AL_cleanup (&al);
        return ec;
    }

    if (al.size != 2) {
        if (dest->error == NULL) {
            char buf[80];
            snprintf (buf, sizeof (buf),
                      "Expected CFArray of length 2, but got %lu",
                      (unsigned long) al.size);
            dest->error = strdup (buf);
            CHECK_NULL (dest->error);
        }

        AL_cleanup (&al);
        return EC_OTHER;
    }

    if (dest->url == NULL) {
        dest->url = strdup (al.strings[0]);
        CHECK_NULL (dest->url);
    }

    if (dest->referrer == NULL) {
        dest->referrer = strdup (al.strings[1]);
        CHECK_NULL (dest->referrer);
    }

    AL_cleanup (&al);
    return EC_OK;
}

ErrorCode getAttributes (const char *fname, Attributes *dest) {
    Attr_init (dest);

    char *result = NULL;
    size_t length = 0;

    ErrorCode ec1 = getAttribute (fname, "com.apple.quarantine",
                                  &result, &length);
    if (ec1 == EC_OK) {
        ec1 = parse_quarantine (dest, result);
    } else if (ec1 != EC_NOATTR) {
        dest->error = result;   /* transfer ownership */
        result = NULL;
    }

    free (result);
    result = NULL;

    if (ec1 != EC_NOFILE) {
        ErrorCode ec2 =
            getAttribute (fname, "com.apple.metadata:kMDItemWhereFroms",
                          &result, &length);
        if (ec2 == EC_OK) {
            ec2 = parse_wherefroms (dest, result, length);
        } else if (ec2 != EC_NOATTR && dest->error == NULL) {
            dest->error = result;   /* transfer ownership */
            result = NULL;
        }

        ec1 = combineErrors (ec1, ec2);

        free (result);
        result = NULL;
    }

    return ec1;
}

#endif  /* __APPLE__ */
