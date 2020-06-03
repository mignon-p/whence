#include "whence.h"

#include <string.h>
#include <stdlib.h>

void split (const char *str, const char *sep, ArrayList *dest) {
    char *s = strdup (str);
    CHECK_NULL (s);

    char *s1 = s;
    char *tok = NULL;

    while (NULL != (tok = strtok (s1, sep))) {
        AL_add (dest, tok);
        s1 = NULL;
    }

    free (s);
}
