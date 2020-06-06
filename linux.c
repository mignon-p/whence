#include "whence.h"

#ifdef __linux__

/* For more information:
 * https://www.freedesktop.org/wiki/CommonExtendedAttributes/
 */

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

#define ATTR(s, f) \
    handle_attribute (fname, (s), &dest->f, &dest->error)

#define A1(s, f) ErrorCode ec = ATTR(s, f)
#define AN(s, f) ec = combineErrors (ec, ATTR(s, f))

ErrorCode getAttributes (const char *fname,
                         Attributes *dest,
                         Cache *cache) {
    A1("user.xdg.origin.url", url);
    AN("user.xdg.referrer.url", referrer);
    AN("user.xdg.origin.email.from", from);
    AN("user.xdg.origin.email.subject", subject);
    AN("user.xdg.origin.email.message-id", message_id);
    AN("user.xdg.publisher", application);

    return ec;
}

#undef AN
#undef A1
#undef ATTR

void Cache_init (Cache *cache) {
    // do nothing
}

void Cache_cleanup (Cache *cache) {
    // do nothing
}

#endif  /* __linux__ */
