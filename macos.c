#include "whence.h"

#ifdef __APPLE__

#include <string.h>
#include <stdio.h>

static void parse_quarantine (Attributes *dest, const char *s) {
    ArrayList al;

    AL_init (&al);
    split (s, ";", &al);

    TODO;
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
                strdup (al.size == 1 ? a.strings[0] : "Unknown error");
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
        parse_quarantine (dest, result);
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

        if (ec2 > ec1) {
            ec1 = ec2;
        }

        free (result);
        result = NULL;
    }

    return ec1;
}

#endif  /* __APPLE__ */
