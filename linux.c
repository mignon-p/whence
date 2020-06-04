#include "whence.h"

#ifdef __linux__

#include <stdlib.h>

static ErrorCode handle_attribute (const char *fname,
                                   const char *aname,
                                   char **dest,
                                   char **error) {
    char *result = NULL;
    size_t length = 0;

    const ErrorCode ec = getAttribute (fname, aname, &result, &length);
    if (ec == EC_OK && *dest == NULL) {
        *dest = result;
    } else if (ec > EC_NOATTR && *error == NULL) {
        *error = result;
    } else {
        free (result);
    }

    return ec;
}

ErrorCode getAttributes (const char *fname, Attributes *dest) {
    ErrorCode ec1 = handle_attribute (fname,
                                      "user.xdg.origin.url",
                                      &dest->url,
                                      &dest->error);
    if (ec1 != EC_NOFILE) {
        const ErrorCode ec2 = handle_attribute (fname,
                                                "user.xdg.referrer.url",
                                                &dest->referrer,
                                                &dest->error);
        ec1 = combineErrors (ec1, ec2);
    }

    return ec1;
}

#endif  /* __linux__ */
