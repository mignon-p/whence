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

char *getErrorString (uint32_t lastErr) {
    char buf[80];

    utf16 *wmsg = NULL;
    const DWORD ret =
        FormatMessageW (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,       /* lpSource */
                        lastErr,    /* dwMessageId */
                        0,          /* dwLanguageId */
                        (void *) &wmsg,
                        1,          /* nSize */
                        NULL);
    if (ret == 0 || wmsg == NULL) {
        snprintf (buf, sizeof (buf),
                  "FormatMessageW failed for code %08X",
                  (unsigned int) lastErr);
        return MY_STRDUP (buf);
    }

    char *msg = utf16to8 (wmsg);
    LocalFree (wmsg);
    if (msg == NULL) {
        snprintf (buf, sizeof (buf),
                  "Failed to convert code %08X to UTF-8",
                  (unsigned int) lastErr);
        return MY_STRDUP (buf);
    }

    return msg;
}

#endif  /* _WIN32 */
