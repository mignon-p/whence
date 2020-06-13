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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <errno.h>

static const char moreinfo[] =
    "For more information see <https://github.com/ppelleti/whence>";

#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
static void get_min_osx (int *major, int *minor) {
    // https://gcc.gnu.org/legacy-ml/gcc-patches/2014-08/msg02428.html
    const int v = __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__;
    if (v < 10000) {
        *major = v / 100;
        *minor = (v / 10) % 10;
    } else {
        *major = v / 10000;
        *minor = (v / 100) % 100;
    }
}
#endif  /* __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ */

static void print_usage (void) {
    fprintf (stderr, "Usage: " CMD_NAME " [OPTIONS] FILE ...\n\n");
    fprintf (stderr, "%-30s%s\n",
             "  -j, --json",
             "Print results in JSON format.");
    fprintf (stderr, "%-30s%s\n",
             "  -h, --help",
             "Print this message and exit.");
    fprintf (stderr, "%-30s%s\n",
             "  -v, --version",
             "Print the version number of " CMD_NAME " and exit.");
    fprintf (stderr, "\n%s\n", moreinfo);
}

static void print_version (void) {
    fprintf (stderr, CMD_NAME " " CMD_VERSION "\n");

    const char* optimize = "";
#ifdef __OPTIMIZE__
    optimize = " (optimized)";
#endif

#ifdef __clang_version__
    fprintf (stderr, "Built with clang %s%s\n", __clang_version__, optimize);
#elif defined (__GNUC__) && defined (__VERSION__)
    fprintf (stderr, "Built with GCC %s%s\n", __VERSION__, optimize);
#endif

#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
    int major, minor;
    get_min_osx (&major, &minor);
    fprintf (stderr, "Built for Mac OS %d.%d and up\n", major, minor);
#endif

#ifdef __APPLE__
    char *sqv = get_sqlite_version ();
    fprintf (stderr, "%s\n", sqv);
    free (sqv);
#endif

    fprintf (stderr, "\n"
             "Copyright (c) 2020 Patrick Pelletier\n"
             "MIT License: <https://en.wikipedia.org/wiki/MIT_License#License_terms>\n"
             "%s\n", moreinfo);
}

static bool is_option (const char *arg, const char *opt1, const char *opt2) {
    if (0 == strcmp (arg, opt1)) {
        return true;
    } else if (0 == strcmp (arg, opt2)) {
        return true;
    } else {
        return false;
    }
}

static int utf8_main (int argc, char **argv) {
    bool json = false;
    int arg1 = 1;

    if (arg1 < argc && is_option (argv[arg1], "-j", "--json")) {
        json = true;
        arg1++;
    }

    if (arg1 < argc && is_option (argv[arg1], "-h", "--help")) {
        print_usage ();
        return EC_OK;
    }

    if (arg1 < argc && is_option (argv[arg1], "-v", "--version")) {
        print_version ();
        return EC_OK;
    }

    const bool colorize = (!json &&
                           isatty (STDOUT_FILENO) &&
                           enableColorEscapes (STDOUT_FILENO));
    colorize_errors =
        isatty (STDERR_FILENO) && enableColorEscapes (STDERR_FILENO);

#ifdef __APPLE__                /* we only format time on MacOS */
    if (!json && NULL == setlocale (LC_TIME, "")) {
        err_printf ("setlocale: %s", strerror (errno));
    }
#endif

    const int nFiles = argc - arg1;

    if (!json && nFiles == 0) {
        err_printf (CMD_NAME ": No files specified on command line");
        print_usage ();
        return EC_CMDLINE;
    }

    Cache cache;
    Cache_init (&cache);

    Attributes attr;
    Attr_init (&attr);

    bool first = true;
    ErrorCode ec = EC_OK;
    int32_t drives = -1;        /* only used on Windows */

    if (json) {
        printf ("{\n");
    }

    for ( ; arg1 < argc; arg1++) {
        char *fname = fixFilename (argv[arg1], &drives);
        const ErrorCode ec2 = getAttributes (fname, &attr, &cache);
        AttrStyle style = (colorize ? AS_HUMAN_COLOR : AS_HUMAN);

        if (json) {
            style = (argc == arg1 + 1 ? AS_JSON_LAST : AS_JSON_NOTLAST);
        }

        Attr_print (&attr, fname, style);
        Attr_cleanup (&attr);

        if (first) {
            ec = ec2;
        } else {
            ec = combineErrors (ec, ec2);
        }

        first = false;
        free (fname);
    }

    if (json) {
        printf ("}\n");
    }

    if (ec == EC_NOATTR && !json) {
        err_printf ("%s: No attributes found",
                    (nFiles == 1 ? argv[argc - 1] : CMD_NAME));
    }

    Cache_cleanup (&cache);
    return ec;
}

#ifdef _WIN32

int wmain (int argc, wchar_t **argv) {
    ArrayList al;

    AL_init (&al);

    int i;
    for (i = 0; i < argc; i++) {
        char *s8 = utf16to8 (argv[i]);
        if (s8 == NULL) {
            err_printf (CMD_NAME ": failed to convert argv[%d] to UTF-8", i);
            AL_cleanup (&al);
            return EC_CMDLINE;
        } else {
            AL_add_nocopy (&al, s8);
        }
    }

    const int ret = utf8_main (argc, al.strings);
    AL_cleanup (&al);

    return ret;
}

#else  /* _WIN32 */

/* assume we have a UTF-8 locale */
int main (int argc, char **argv) {
    return utf8_main (argc, argv);
}

#endif  /* _WIN32 */
