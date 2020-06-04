#include "whence.h"

#include <string.h>
#include <stdlib.h>

void split (const char *str, char sep, ArrayList *dest) {
    const char *s = str;

    for ( ; ; ) {
        const char *p = strchr (s, sep);
        if (p == NULL) {
            AL_add (dest, s);
            break;
        } else {
            const size_t len = p - s;
            char *newStr = malloc (len + 1);
            CHECK_NULL (newStr);
            memcpy (newStr, s, len);
            newStr[len] = 0;
            AL_add_nocopy (dest, newStr);
            s = p + 1;
        }
    }
}
