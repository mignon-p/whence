#include "whence.h"

#ifdef _WIN32

#include <Windows.h>
#include <io.h>

static DWORD oldMode;
static HANDLE consoleHandle;

static void restore_mode (void) {
    SetConsoleMode (consoleHandle, oldMode);
}

bool enableColorEscapes (int fd) {
    const HANDLE h = _get_osfhandle (fd);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }

    if (! GetConsoleMode (h, &oldMode)) {
        return false;
    }

    consoleHandle = h;
    if (0 != atexit (restore_mode)) {
        return false;
    }

    return SetConsoleMode (h, oldMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

#else  /* _WIN32 */

bool enableColorEscapes (int fd) {
    return true;
}

#endif  /* _WIN32 */
