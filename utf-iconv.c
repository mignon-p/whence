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

#include <iconv.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef enum Opened {
    Opened_NotTried,
    Opened_Succeeded,
    Opened_Failed
} Opened;

typedef union TestEndian {
    uint16_t u16;
    uint8_t u8[2];
} TestEndian;

static iconv_t conv_handle;     /* for converting utf-8 to utf-16 */
static Opened conv_opened = Opened_NotTried;

static const char *utf16_name (void) {
    TestEndian u;

    u.u16 = 0x1234;
    if (u.u8[0] == 0x34) {
        return "UTF-16LE";
    } else {
        return "UTF-16BE";
    }
}

static void close_conv (void) {
    iconv_close (conv_handle);
    conv_opened = Opened_NotTried;
}

static bool open_conv (void) {
    if (conv_opened == Opened_NotTried) {
        conv_handle = iconv_open (utf16_name(), "UTF-8");
        /*                        ^to           ^from */

        if (conv_handle == (iconv_t)(-1)) {
            conv_opened = Opened_Failed;
            err_printf ("iconv_open: %s", strerror (errno));
        } else {
            conv_opened = Opened_Succeeded;
            atexit (close_conv);
        }
    }

    return (conv_opened == Opened_Succeeded);
}

utf16 *utf8to16_len (const char *s, size_t len) {
    if (len == 0) {             /* special case for empty string */
        utf16 *result = malloc (2);
        CHECK_NULL (result);
        *result = 0;
        return result;
    }

    if (! open_conv()) {
        return NULL;
    }

    const size_t maxlen = 2 * (len + 1);
    utf16 *result = (utf16 *) malloc (maxlen);
    CHECK_NULL (result);

    char *inbuf = (char *) s;   /* cast away const; should be safe */
    char *outbuf = (char *) result;
    size_t inbytesleft = len;
    size_t outbytesleft = maxlen;

    const size_t ret = iconv (conv_handle,
                              &inbuf, &inbytesleft,
                              &outbuf, &outbytesleft);
    if (ret == (size_t)(-1)) {
        free (result);
        return NULL;
    }

    const size_t bytes_converted = outbuf - (char *) result;
    if (bytes_converted >= maxlen || (bytes_converted & 1) != 0) {
        free (result);
        return NULL;
    }

    /* NUL terminate the UTF-16 string */
    result[bytes_converted / 2] = 0;
    /* Shrink allocated buffer to size used */
    result = (utf16 *) realloc (result, bytes_converted + 2);
    CHECK_NULL (result);
    return result;
}

/* currently unimplemented:
 *   utf16 *utf8to16 (const char *s);
 *   char *utf16to8 (const utf16 *s);
 *   char *utf16to8_len (const utf16 *s, size_t len);
 */

#endif  /* not _WIN32 */
