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

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
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
    utf16* wStreamName = utf8to16_nofail (streamName);
    utf16* wfname = NULL;

    FILE *f = _wfopen (wStreamName, L"r");
    if (!f) {
        const int errnum = errno;
        *result = MY_STRDUP (strerror (errnum));
        *length = strlen (*result);

        wfname = utf8to16_nofail (fname);

        if (_waccess (wfname, 0) == 0) {
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

    free (wfname);
    free (wStreamName);
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

static bool haveDrive (char drive, int32_t *drives) {
    const int n = drive - 'A';

    if (n < 0 || n >= 26) {
        return false;
    }

    if (*drives == -1) {
        *drives = GetLogicalDrives ();
    }

    return (((*drives >> n) & 1) != 0);
}

/* See comment about fixFilename() in whence.h for a description
 * of the two problems that this function fixes.
 */
char *fixFilename (const char *fname, int32_t *drives) {
    char *s = MY_STRDUP (fname);

    /* Problem #1 */
    if (s[0] == '/' && isalpha (s[1]) && s[2] == '/') {
        if (_access (s, 0) != 0 && errno == ENOENT) {
            const char drive = toupper (s[1]);
            if (haveDrive (drive, drives)) {
                s[0] = drive;
                s[1] = ':';
                if (_access (s, 0) != 0) {
                    /* If we still can't access the file, restore the
                     * original filename. */
                    s[0] = fname[0];
                    s[1] = fname[1];
                }
            }
        }
    }

    /* Problem #2 */
    if (isalnum(s[0]) && s[1] == 0) {
        s = realloc (s, 4);
        CHECK_NULL (s);
        s[0] = '.';
        s[1] = '/';
        s[2] = fname[0];
        s[3] = 0;
    }

    return s;
}

#endif  /* _WIN32 */
