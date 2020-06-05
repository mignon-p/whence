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
        *result = MY_STRDUP (strerror (errnum));
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
        char buf[81];

        const size_t bites = fread (buf, 1, sizeof (buf) - 1, f);
        if (bites == 0 || bites >= sizeof (buf)) {
            break;
        } else {
            buf[bites] = 0;     /* NUL terminate string */
            AL_add (&al, buf);
        }
    }

    if (ferror (f)) {
        *result = MY_STRDUP (strerror (errno));
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

static int handleKey (const char *key,
                      const char *value,
                      Attributes *dest,
                      ZoneCache *zc) {
    char **field = NULL;

    if (0 == strcmp (key, "ReferrerUrl")) {
        field = &dest->referrer;
    } else if (0 == strcmp (key, "HostUrl")) {
        field = &dest->url;
    } else if (0 == strcmp (key, "ZoneId")) {
        field = &dest->zone;
        value = getZoneName (value, zc);
    }

    if (field) {
        *field = MY_STRDUP (value);
        return 1;
    } else {
        return 0;
    }
}

static int parseZoneIdentifier (const char *zi,
                                Attributes *dest,
                                ZoneCache *zc) {
    int count = 0;
    ArrayList lines;

    AL_init (&lines);
    split (zi, '\n', &lines);

    size_t i;
    for (i = 0; i < lines.size; i++) {
        char *line = lines.strings[i];
        char *eq = strchr (line, '=');
        if (eq) {
            *eq = 0;
            count += handleKey (line, eq + 1, dest, zc);
            *eq = '=';
        }
    }

    AL_cleanup (&lines);
    return count;
}

ErrorCode getAttributes (const char *fname,
                         Attributes *dest,
                         ZoneCache *zc) {
    char *result = NULL;
    size_t length = 0;

    const ErrorCode ec =
        getAttribute (fname, "Zone.Identifier", &result, &length);
    if (ec > EC_NOATTR && dest->error == NULL) {
        dest->error = result;
        return ec;
    } else if (ec != EC_OK) {
        free (result);
        return ec;
    }

    const int numAttrs = parseZoneIdentifier (result, dest, zc);
    free (result);
    return (numAttrs == 0 ? EC_NOATTR : EC_OK);
}

#endif  /* _WIN32 */
