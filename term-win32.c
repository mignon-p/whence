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
#include <stdlib.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 4
#endif

typedef struct RestoreMode {
    HANDLE h;
    DWORD mode;
    struct RestoreMode *next;
} RestoreMode;

static RestoreMode *oldModes = NULL;

Terminal stdoutTerminal;
Terminal stderrTerminal;
static bool initialized = false;

static void add_mode (HANDLE h, DWORD mode) {
    RestoreMode *r;

    for (r = oldModes; r != NULL; r = r->next) {
        if (r->h == h) {
            return;             /* already saved mode for this handle */
        }
    }

    r = malloc (sizeof (*r));
    CHECK_NULL (r);
    r->h = h;
    r->mode = mode;
    r->next = oldModes;
    oldModes = r;
}

static void restore_mode (void) {
    while (oldModes != NULL) {
        SetConsoleMode (oldModes->h, oldModes->mode);
        RestoreMode *next = oldModes->next;
        free (oldModes);
        oldModes = next;
    }
}

static bool save_mode (int fd, Terminal *t) {
    if (! isatty(fd)) {
        t->is_terminal = false;
        t->supports_color = false;
        return;
    }

    const HANDLE h = (HANDLE) _get_osfhandle (fd);
    if (h == INVALID_HANDLE_VALUE) {
        t->is_terminal = false;
        t->supports_color = false;
        return;
    }

    DWORD m = 0;
    if (! GetConsoleMode (h, &m)) {
        t->is_terminal = false;
        t->supports_color = false;
        return;
    }

    t->is_terminal = true;

    if (0 != (m & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        t->supports_color = true; /* already enabled */
        return;
    }

    const bool changedMode =
        SetConsoleMode (h, m | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    t->supports_color = changedMode;

    if (changedMode) {
        add_mode (h, m);
    }
}

void detectConsole (void) {
    if (! initialized) {
        save_mode (STDOUT_FILENO, &stdoutTerminal);
        save_mode (STDERR_FILENO, &stderrTerminal);

        if (oldModes != NULL) {
            atexit (restore_mode);
        }

        initialized = true;
    }
}

void writeUTF8 (FILE *f, const char *s) {
    const int fd = fileno (f);
    bool useConsole;

    switch (fd) {
    case STDOUT_FILENO: useConsole = stdoutTerminal.is_terminal; break;
    case STDERR_FILENO: useConsole = stderrTerminal.is_terminal; break;
    default:            useConsole = false;                      break;
    }

    if (useConsole) {
        const HANDLE h = (HANDLE) _get_osfhandle (fd);
        if (h == INVALID_HANDLE_VALUE) {
            goto narrow;
        }

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
                if (w == wide) {
                    free (wide);
                    goto narrow;
                } else {
                    break;
                }
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

#endif  /* _WIN32 */
