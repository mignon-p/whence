#include <stdio.h>

#include "whence.h"

int main (int argc, char **argv) {
    if (argc > 1) {
        char *result;
        size_t length;
        const ErrorCode ec = getAttribute (argv[1], "com.apple.quarantine", &result, &length);
        if (ec == EC_OK) {
            printf ("%s\n", result);
        } else if (ec == EC_MEM) {
            fprintf (stderr, "Out of memory\n");
        } else {
            fprintf (stderr, "%s\n", result);
        }
        return ec;
    }

    return EC_CMDLINE;
}
