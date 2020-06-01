#include "whence.h"

#include <stdio.h>
#include <stdlib.h>

#if 0
int main (int argc, char **argv) {
    if (argc > 1) {
        char *result = NULL;
        size_t length = 0;

        const ErrorCode ec1 = getAttribute (argv[1], "com.apple.quarantine",
                                            &result, &length);
        if (ec1 == EC_OK) {
            printf ("%s\n", result);
        } else if (ec1 == EC_MEM) {
            fprintf (stderr, "Out of memory\n");
        } else {
            fprintf (stderr, "%s\n", result);
        }

        free (result);
        result = NULL;

        const ErrorCode ec2 =
            getAttribute (argv[1], "com.apple.metadata:kMDItemWhereFroms",
                          &result, &length);
        if (ec2 == EC_OK) {
            printProps (result, length);
        } else if (ec2 == EC_MEM) {
            fprintf (stderr, "Out of memory\n");
        } else {
            fprintf (stderr, "%s\n", result);
        }

        free (result);

        if (ec1 > ec2) {
            return ec1;
        } else {
            return ec2;
        }
    }

    return EC_CMDLINE;
}
#endif

int main (int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        const char *filename = argv[i];
        char *result = NULL;
        size_t length = 0;

        const ErrorCode ec2 =
            getAttribute (argv[1], "com.apple.metadata:kMDItemWhereFroms",
                          &result, &length);
        if (ec2 == EC_OK) {
            checkProps (result, length, filename);
        }

        free (result);
    }

    return 0;
}
