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
}

static void print_version (void) {
    fprintf (stderr, CMD_NAME " 0.9\n");
#ifdef __clang_version__
    fprintf (stderr, "Built with clang %s\n", __clang_version__);
#elif defined (__GNUC__) && defined (__VERSION__)
    fprintf (stderr, "Built with GCC %s\n", __VERSION__);
#endif
    fprintf (stderr,
             "Copyright (c) 2020 Patrick Pelletier\n"
             "MIT License: <https://en.wikipedia.org/wiki/MIT_License#License_terms>\n"
             "For more information see <https://github.com/ppelleti/whence>\n");
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

int main (int argc, char **argv) {
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
