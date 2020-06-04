#include "whence.h"

#ifdef _WIN32

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

ErrorCode getAttribute (const char *fname,
                        const char *attr,
                        char **result,
                        size_t *length) {
    ArrayList al;

    AL_init (&al);
    AL_add (&al, fname);
    AL_add (&al, ":");
    AL_add (&al, attr);
    AL_add (&al, ":$DATA");

    char *streamName = AL_join (&al);
    AL_clear (&al);

    ErrorCode ec = EC_OK;

    FILE *f = fopen (streamName, "r");
    if (!f) {
        const int errnum = errno;
        *result = strdup (strerror (errnum));
        CHECK_NULL (*result);
        *length = strlen (*result);

        if (_access (fname, 0) == 0) {
            ec = EC_NOATTR;     /* file exists but alternate stream does not */
            goto done;
        } else {
            if (errno == ENOENT) {
                ec = EC_NOFILE;
            } else {
                ec = EC_OTHER;
            }

            goto done;
        }
    }

    for ( ; ; ) {
        char buf[32];

        const size_t bites = fread (buf, 1, sizeof (buf) - 1, f);
        if (bites == 0 || bites >= sizeof (buf)) {
            break;
        } else {
            buf[bites] = 0;     /* NUL terminate string */
            AL_add (&al, buf);
        }
    }

    if (ferror (f)) {
        *result = strdup (strerror (errno));
        CHECK_NULL (*result);
        *length = strlen (*result);
        ec = EC_OTHER;
        goto done;
    }

    *result = AL_join (&al);
    *length = strlen (*result);

 done:
    if (f != NULL) {
        fclose (f);
    }

    free (streamName);
    AL_cleanup (&al);

    return ec;
}

int main (int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        const char *fname = argv[i];
        char *result = NULL;
        size_t length = 0;

        const ErrorCode ec =
            getAttribute (fname, "Zone.Identifier", &result, &length);
        if (ec == EC_OK) {
            printf ("%s:\n%s\n", fname, result);
        } else {
            printf ("%s: %s\n", fname, result);
        }

        free (result);
    }

    return 0;
}

#endif  /* _WIN32 */
