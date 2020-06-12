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

#ifdef __APPLE__

#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

void MyDate_clear (MyDate *date) {
    memset (date, 0, sizeof (*date));
}

void MyDate_set_integer (MyDate *date, time_t t) {
    date->seconds = t;
    date->milliseconds = 0;
    date->secondsValid = true;
    date->millisValid = false;
}

void MyDate_set_fractional (MyDate *date, double t) {
    const double seconds = floor (t);
    const double frac = t - seconds;
    date->seconds = (time_t) seconds;
    date->milliseconds = (uint16_t) (frac * 1000);
    date->secondsValid = true;
    date->millisValid = true;
}

char *MyDate_format_human (const MyDate *date) {
    const time_t t = date->seconds;
    char buf[40];

    strftime (buf, sizeof (buf), "%+", localtime (&t));
    return MY_STRDUP (buf);
}

char *MyDate_format_iso8601 (const MyDate *date) {
    const time_t t = date->seconds;
    char buf1[40], buf2[40];

    strftime (buf1, sizeof (buf1), "%Y-%m-%dT%H:%M:%S", gmtime (&t));
    if (date->millisValid) {
        snprintf (buf2, sizeof (buf2),
                  "%s.%03uZ", buf1, (unsigned int) date->milliseconds);
    } else {
        snprintf (buf2, sizeof (buf2), "%sZ", buf1);
    }

    return MY_STRDUP (buf2);
}

#endif  /* __APPLE__ */
