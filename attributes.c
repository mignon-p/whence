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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define TRUNCATION_LIMIT 1600

typedef struct PrCtx {
    bool empty;
    bool colorize;
    bool firstField;
    bool lastFile;
} PrCtx;

typedef struct Printer {
    void (*print_fname) (const char *fname, PrCtx *ctx);
    void (*print_field) (const char *field,
                         const char *value,
                         PrCtx *ctx);
    void (*print_end) (PrCtx *ctx);
} Printer;

static void PrCtx_init (PrCtx *ctx) {
    memset (ctx, 0, sizeof (*ctx));
    ctx->firstField = true;
}

static void print_limited (const char *s, bool useColor) {
    const size_t len = strlen (s);

    if (len <= TRUNCATION_LIMIT) {
        writeUTF8 (stdout, s);
        printf ("\n");
    } else {
        char buf[TRUNCATION_LIMIT + 1];

        memcpy (buf, s, TRUNCATION_LIMIT);
        buf[TRUNCATION_LIMIT] = 0;

        writeUTF8 (stdout, buf);
        setColor (stdout, useColor, COLOR_RED);
        printf ("... (%lu bytes)", (unsigned long) len);
        setColor (stdout, useColor, COLOR_OFF);
        printf ("\n");
    }
}

static void human_print_fname (const char *fname, PrCtx *ctx) {
    if (! ctx->empty) {
        setColor (stdout, ctx->colorize, COLOR_MAGENTA);
        writeUTF8 (stdout, fname);
        setColor (stdout, ctx->colorize, COLOR_OFF);
        printf (":\n");
    }
}

static void human_print_field (const char *field,
                               const char *value,
                               PrCtx *ctx) {
    setColor (stdout, ctx->colorize, COLOR_GREEN);
    printf ("  %-11s ", field);
    setColor (stdout, ctx->colorize, COLOR_OFF);
    print_limited (value, ctx->colorize);
}

static void human_print_end (PrCtx *ctx) {
    /* do nothing */
}

static void print_string (const char *s, bool forceLC) {
    putchar ('"');

    char c;
    while (0 != (c = *(s++))) {
        if (forceLC) {
            c = tolower (c);
        }

        const unsigned char uc = (unsigned char) c;

        if (c == '"') {
            printf ("\\\"");
        } else if (c == '\\') {
            printf ("\\\\");
        } else if (uc < 0x20 || uc == 0x7f) {
            printf ("\\u%04X", c);
        } else if (uc >= 0x80) {
            s--;
            s += print_escaped_unicode (s);
        } else {
            putchar (c);
        }
    }

    putchar ('"');
}

static void json_print_fname (const char *fname, PrCtx *ctx) {
    printf ("  ");
    print_string (fname, false);
    printf (": {");
}

static void json_print_field (const char *field,
                              const char *value,
                              PrCtx *ctx) {
    if (ctx->firstField == true) {
        ctx->firstField = false;
    } else {
        printf (",");
    }

    printf ("\n    ");
    print_string (field, true);
    printf (": ");
    print_string (value, false);
}

static void json_print_end (PrCtx *ctx) {
    printf ("\n  }");
    if (ctx->lastFile) {
        printf ("\n");
    } else {
        printf (",\n");
    }
}

static const Printer printer_human = {
    human_print_fname,
    human_print_field,
    human_print_end
};

static const Printer printer_json = {
    json_print_fname,
    json_print_field,
    json_print_end
};

static const Printer *get_printer (AttrStyle style) {
    switch (style) {
    case AS_HUMAN:
    case AS_HUMAN_COLOR:
        return &printer_human;
    default:
        return &printer_json;
    }
}

static bool is_json (AttrStyle style) {
    switch (style) {
    case AS_JSON_NOTLAST:
    case AS_JSON_LAST:
        return true;
    default:
        return false;
    }
}

static bool isEmpty (const Attributes *attrs) {
    const unsigned char *p = (unsigned char *) attrs;
    size_t i;

    for (i = 0; i < sizeof (*attrs); i++) {
        if (p[i] != 0) {
            return false;
        }
    }

    return true;
}

void Attr_init (Attributes *attrs) {
    memset (attrs, 0, sizeof (*attrs));
}

#define PR(field, value) \
    if (value) p->print_field (field, value, &ctx)

void Attr_print (const Attributes *attrs, const char *fname, AttrStyle style) {
    const Printer *p = get_printer (style);
    const bool lastFile = (style == AS_JSON_LAST);

    PrCtx ctx;
    PrCtx_init (&ctx);
    ctx.lastFile = lastFile;

    if (attrs->error != NULL && !is_json (style)) {
        err_printf ("%s: %s", fname, attrs->error);
        return;
    }

    char *date = NULL;

    /* only MacOS records the date that the file was downloaded */
#ifdef __APPLE__
    if (attrs->date.secondsValid) {
        if (is_json (style)) {
            date = MyDate_format_iso8601 (&attrs->date);
        } else {
            date = MyDate_format_human (&attrs->date);
        }
    }
#endif

    ctx.empty = isEmpty (attrs);
    ctx.colorize = (style == AS_HUMAN_COLOR);

    p->print_fname (fname, &ctx);
    PR("URL", attrs->url);
    PR("Referrer", attrs->referrer);
    PR("From", attrs->from);
    PR("Subject", attrs->subject);
    PR("Message-ID", attrs->message_id);
    PR("Application", attrs->application);
    PR("Date", date);
    PR("Zone", attrs->zone);
    PR("Error", attrs->error);
    p->print_end (&ctx);

    free (date);
}

#undef PR

void Attr_cleanup (Attributes *attrs) {
    free (attrs->url);
    free (attrs->referrer);
    free (attrs->from);
    free (attrs->subject);
    free (attrs->message_id);
    free (attrs->application);
    free (attrs->zone);
    free (attrs->error);
    Attr_init (attrs);
}
