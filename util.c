#include "whence.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void oom (const char *file, long line) {
    const char *slash = strrchr (file, '/');
    if (slash) {
        file = slash + 1;       /* just the basename */
    }

    fprintf (stderr, "whence: out of memory at %s:%ld\n", file, line);
    exit (EC_MEM);
}
