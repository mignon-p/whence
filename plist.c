#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFNumberFormatter.h>

#include "whence.h"

void printPlist (CFPropertyListRef plist, uintptr_t indent);

static void printString (CFStringRef str) {
    size_t buflen = CFStringGetLength (str) * 4 + 1;
    char *buf = malloc (buflen);
    if (buf) {
        if (CFStringGetCString (str, buf, buflen, kCFStringEncodingUTF8)) {
            printf ("%s", buf);
        }
        free (buf);
    }
}

static void printNumber (CFNumberRef num) {
    CFLocaleRef loc = CFLocaleCreate (NULL, CFSTR("C"));
    if (! loc) {
        return;
    }
    CFNumberFormatterRef f =
        CFNumberFormatterCreate (NULL, NULL, kCFNumberFormatterNoStyle);
    if (f) {
        CFStringRef str =
            CFNumberFormatterCreateStringWithNumber (NULL, f, num);
        if (str) {
            printString (str);
            CFRelease (str);
        }
        CFRelease (f);
    }
    CFRelease (loc);
}

static void arrApp (const void *value, void *context) {
    printPlist (value, (uintptr_t) context);
}

static void printArray (CFArrayRef a, uintptr_t indent) {
    const CFIndex count = CFArrayGetCount (a);
    CFArrayApplyFunction (a, CFRangeMake (0, count), arrApp, (void *) indent);
}

void printPlist (CFPropertyListRef plist, uintptr_t indent) {
    uintptr_t i;

    for (i = 0; i < indent; i++) {
        printf ("  ");
    }

    if (plist == NULL) {
        printf ("NULL\n");
        return;
    }

    const CFTypeID t = CFGetTypeID (plist);

    if (t == CFStringGetTypeID()) {
        printf ("CFString ");
        printString (plist);
        printf ("\n");
    } else if (t == CFBooleanGetTypeID()) {
        const Boolean b = CFBooleanGetValue (plist);
        printf ("CFBoolean %s\n", b ? "true" : "false");
    } else if (t == CFNumberGetTypeID()) {
        printf ("CFNumber ");
        printNumber (plist);
        printf ("\n");
    } else if (t == CFArrayGetTypeID()) {
        const unsigned long count = CFArrayGetCount (plist);
        printf ("CFArray %lu\n", count);
        printArray (plist, indent + 1);
    } else if (t == CFDictionaryGetTypeID()) {
        printf ("CFDictionary\n");
    } else if (t == CFDateGetTypeID()) {
        printf ("CFDate\n");
    } else if (t == CFDataGetTypeID()) {
        printf ("CFData\n");
    } else {
        printf ("Unknown\n");
    }
}

ErrorCode printProps (const void *data, size_t length) {
    CFDataRef d = CFDataCreate (NULL, data, length);
    if (d == NULL) {
        return EC_MEM;
    }

    CFErrorRef err = NULL;
    CFPropertyListRef plist =
        CFPropertyListCreateWithData (NULL, d, kCFPropertyListImmutable,
                                      NULL, &err);
    CFRelease (d);
    if (plist == NULL) {
        CFStringRef msg = CFErrorCopyDescription (err);
        printString (msg);
        printf ("\n");
        CFRelease (msg);
        CFRelease (err);
        return EC_OTHER;
    }

    printPlist (plist, 0);
    CFRelease (plist);
    return EC_OK;
}
