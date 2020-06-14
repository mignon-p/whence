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
#include <io.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 4
#endif

static HANDLE consoleHandles[] =
    { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
#define NUM_STD_FDS (sizeof (consoleHandles) / sizeof (consoleHandles[0]))
static DWORD oldModes[NUM_STD_FDS];

static bool needAtExit = true;

static void restore_mode (void) {
    size_t i;

    for (i = 0; i < NUM_STD_FDS; i++) {
        if (consoleHandles[i] != INVALID_HANDLE_VALUE) {
            SetConsoleMode (consoleHandles[i], oldModes[i]);
        }
    }
}

bool detectConsole (FILE *f) {
    const int fd = fileno (f);

    if (fd >= NUM_STD_FDS) {
        return false;
    }

    if (! isatty(fd)) {
        return false;
    }

    if (consoleHandles[fd] != INVALID_HANDLE_VALUE) {
        return true;            /* already called on this fd */
    }

    const HANDLE h = (HANDLE) _get_osfhandle (fd);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD m = 0;
    if (! GetConsoleMode (h, &m)) {
        return false;
    }

    if (0 != (m & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        return true;            /* already enabled */
    }

    consoleHandles[fd] = h;
    oldModes[fd] = m;
    if (needAtExit && 0 != atexit (restore_mode)) {
        return false;
    }

    needAtExit = false;

    return SetConsoleMode (h, m | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void writeUTF8 (FILE *f, const char *s) {
    const int fd = fileno (f);

    if (fd < NUM_STD_FDS && consoleHandles[fd] != INVALID_HANDLE_VALUE) {
        const HANDLE h = consoleHandles[fd];
        utf16 *wide = utf8to16 (s);
        if (wide == NULL) {
            goto narrow;
        }

        fflush (f);

        size_t len = wcslen (wide);
        utf16 *w = wide;

        while (len > 0) {
            DWORD written = 0;
            if (! WriteConsoleW (h, w, len, &written, NULL)) {
                break;
            }

            len -= written;
            w += written;
        }

        free (wide);
    } else {
    narrow:
        fputs (s, f);
    }
}

#else  /* _WIN32 */

#include <unistd.h>
#include <stdio.h>

bool detectConsole (FILE *f) {
    return isatty (fileno (f));
}

void writeUTF8 (FILE *f, const char *s) {
    fputs (s, f);
}

#endif  /* _WIN32 */
