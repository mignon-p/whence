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

bool enableColorEscapes (int fd) {
    if (fd >= NUM_STD_FDS) {
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

#else  /* _WIN32 */

bool enableColorEscapes (int fd) {
    return true;
}

#endif  /* _WIN32 */
