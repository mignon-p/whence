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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static const char *my_basename (const char *file) {
    const char *slash = strrchr (file, '/');
    if (slash) {
        return slash + 1;
    } else {
        return file;
    }
}

void oom (const char *file, long line) {
    err_printf (CMD_NAME ": out of memory at %s:%ld",
                my_basename (file), line);
    exit (EC_MEM);
}

ErrorCode combineErrors (ErrorCode ec1, ErrorCode ec2) {
    if (ec2 > ec1 && ec2 > EC_NOATTR) {
        return ec2;
    } else if (ec1 > EC_NOATTR) {
        return ec1;
    } else if (ec2 == EC_OK) {
        return EC_OK;
    } else {
        return ec1;
    }
}

char *my_strdup (const char *s, const char *file, long line) {
    if (s == NULL) {
        err_printf (CMD_NAME ": strdup called on NULL at %s:%ld",
                    my_basename (file), line);
        exit (EC_OTHER);
    }

    char *ret = strdup (s);
    if (ret == NULL) {
        oom (file, line);
    }

    return ret;
}

void err_printf (const char *format, ...) {
    setColor (stderr, stderrTerminal.supports_color, COLOR_RED);

    va_list va;
    va_start (va, format);
    vfprintf (stderr, format, va);
    va_end (va);

    setColor (stderr, stderrTerminal.supports_color, COLOR_OFF);
    fprintf (stderr, "\n");
}

size_t print_escaped_unicode (const char *s) {
    const char *p;

    for (p = s; ((unsigned char) (*p)) >= 0x80; p++) {
        /* empty */
    }

    const size_t len = p - s;
    utf16 *wide = utf8to16_len (s, len);

    if (wide == NULL) {
        /* If the conversion to UTF-16 fails (e. g. invalid UTF-8)
         * then just print out the alleged UTF-8 as-is, and hope
         * that's okay. */
        size_t i;

        for (i = 0; i < len; i++) {
            putchar (s[i]);
        }
    } else {
        utf16 *pw;

        for (pw = wide; *pw != 0; pw++) {
            const unsigned int c = *pw;
            printf ("\\u%04X", c & 0xffff);
        }

        free (wide);
    }

    return len;
}

void setColor (FILE *f, bool useColor, int color) {
    if (useColor) {
        fprintf (f, "\e[%dm", color);
    }
}

bool envNoColor (void) {
    const char *nocolor = getenv ("NO_COLOR");

    if (nocolor && *nocolor) {
        return true;
    } else {
        return false;
    }
}
