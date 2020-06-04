#include "whence.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

typedef struct Printer {
    void (*print_fname) (const char *fname);
    void (*print_field) (const char *field,
                         const char *value,
                         bool *firstField);
    void (*print_end) (bool lastFile);
} Printer;

static void human_print_fname (const char *fname) {
    printf ("%s:\n", fname);
}

static void human_print_field (const char *field,
                               const char *value,
                               bool *firstField) {
    printf ("  %-11s %s\n", field, value);
}

static void human_print_fname_color (const char *fname) {
    printf ("\e[95m%s\e[0m:\n", fname);
}

static void human_print_field_color (const char *field,
                                     const char *value,
                                     bool *firstField) {
    printf ("  \e[92m%-11s\e[0m %s\n", field, value);
}

static void human_print_end (bool lastFile) {
    /* do nothing */
}

static void print_string (const char *s, bool forceLC) {
    putchar ('"');

    char c;
    while (0 != (c = *(s++))) {
        if (forceLC) {
            c = tolower (c);
        }

        if (c == '"') {
            printf ("\\\"");
        } else if (c == '\\') {
            printf ("\\\\");
        } else if (c < ' ') {
            printf ("\\u%04X", c);
        } else {
            putchar (c);
        }
    }

    putchar ('"');
}

static void json_print_fname (const char *fname) {
    printf ("  ");
    print_string (fname, false);
    printf (": {");
}

static void json_print_field (const char *field,
                              const char *value,
                              bool *firstField) {
    if (*firstField == true) {
        *firstField = false;
    } else {
        printf (",");
    }

    printf ("\n    ");
    print_string (field, true);
    printf (": ");
    print_string (value, false);
}

static void json_print_end (bool lastFile) {
    printf ("\n  }");
    if (lastFile) {
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

static const Printer printer_human_color = {
    human_print_fname_color,
    human_print_field_color,
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
        return &printer_human;
    case AS_HUMAN_COLOR:
        return &printer_human_color;
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

void Attr_init (Attributes *attrs) {
    memset (attrs, 0, sizeof (*attrs));
}

#define PR(field, value) \
    if (value) p->print_field (field, value, &firstField)

void Attr_print (const Attributes *attrs, const char *fname, AttrStyle style) {
    const Printer *p = get_printer (style);
    const bool lastFile = (style == AS_JSON_LAST);
    bool firstField = true;

    if (attrs->error != NULL && !is_json (style)) {
        fprintf (stderr, "%s: %s\n", fname, attrs->error);
        return;
    }

    char buf[40];
    const char *date = NULL;
    const time_t t = attrs->date;
    if (t != 0) {
        if (!is_json (style)) {
#ifdef __linux__
            strftime (buf, sizeof (buf), "%c", localtime (&t));
#else
            strftime (buf, sizeof (buf), "%+", localtime (&t));
#endif
        } else {
            strftime (buf, sizeof (buf), "%Y-%m-%dT%H:%M:%SZ", gmtime (&t));
        }

        date = buf;
    }

    p->print_fname (fname);
    PR("URL", attrs->url);
    PR("Referrer", attrs->referrer);
    PR("Application", attrs->application);
    PR("Date", date);
    PR("Error", attrs->error);
    p->print_end (lastFile);
}

#undef PR

void Attr_cleanup (Attributes *attrs) {
    free (attrs->url);
    free (attrs->referrer);
    free (attrs->application);
    free (attrs->error);
}
