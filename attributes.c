#include "whence.h"

#include <stdlib.h>

typedef struct Printer {
    void (*print_fname) (const char *fname);
    void (*print_field) (const char *field,
                         const char *value,
                         bool *firstField,
                         bool error);
    void (*print_end) (bool lastFile);
} Printer;

static void human_print_fname (const char *fname) {
}

static void human_print_field (const char *field,
                               const char *value,
                               bool *firstField,
                               bool error) {
}

static void human_print_end (bool lastFile) {
}

static void json_print_fname (const char *fname) {
}

static void json_print_field (const char *field,
                              const char *value,
                              bool *firstField,
                              bool error) {
}

static void json_print_end (bool lastFile) {
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

void Attr_init (Attributes *attrs) {
    memset (attrs, 0, sizeof (*attrs));
}

#define PR(field, value, error) \
    if (value) p->print_field (field, value, &firstField, error)

void Attr_print (const Attributes *attrs, const char *fname, AttrStyle style) {
    const Printer *p = (style == AS_HUMAN ? printer_human : printer_json);
    const bool lastFile = (style == AS_JSON_LAST);
    bool firstField = true;

    p->print_fname (fname);
    PR("URL", attrs->url, false);
    PR("Referrer", attrs->referrer, false);
    PR("Application", attrs->application, false);
    PR("Date", date, false);
    PR("Error", attrs->error, true);
    p->print_end (lastFile);
}

void Attr_cleanup (Attributes *attrs) {
    free (attrs->url);
    free (attrs->referrer);
    free (attrs->application);
    free (attrs->error);
}
