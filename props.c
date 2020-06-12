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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFString.h>

typedef struct ArrAppContext {
    ArrayList *dest;
    unsigned long count;
    ErrorCode ec;
} ArrAppContext;

typedef struct DateContext {
    MyDate *date;
    char **errmsg;
    unsigned long count;
} DateContext;

static const char *typeString (CFPropertyListRef plist) {
    const CFTypeID t = CFGetTypeID (plist);

    if (t == CFStringGetTypeID()) {
        return "CFString";
    } else if (t == CFBooleanGetTypeID()) {
        return "CFBoolean";
    } else if (t == CFNumberGetTypeID()) {
        return "CFNumber";
    } else if (t == CFArrayGetTypeID()) {
        return "CFArray";
    } else if (t == CFDictionaryGetTypeID()) {
        return "CFDictionary";
    } else if (t == CFDateGetTypeID()) {
        return "CFDate";
    } else if (t == CFDataGetTypeID()) {
        return "CFData";
    } else {
        return "Unknown";
    }
}

static void addString (CFStringRef str, ArrayList *dest) {
    size_t buflen = CFStringGetLength (str) * 4 + 1;
    char *buf = malloc (buflen);
    CHECK_NULL (buf);
    if (CFStringGetCString (str, buf, buflen, kCFStringEncodingUTF8)) {
        AL_add (dest, buf);
    } else {
        AL_add (dest, "???");
    }
    free (buf);
}

static char *copyString (CFStringRef str) {
    size_t buflen = CFStringGetLength (str) * 4 + 1;
    char *buf = malloc (buflen);
    CHECK_NULL (buf);
    if (CFStringGetCString (str, buf, buflen, kCFStringEncodingUTF8)) {
        const size_t len = strlen (buf);
        buf = realloc (buf, len + 1);
        CHECK_NULL (buf);
        return buf;
    } else {
        buf = realloc (buf, 4);
        CHECK_NULL (buf);
        memset (buf, '?', 3);
        buf[3] = 0;
        return buf;
    }
}

static void arrApp (const void *value, void *context) {
    ArrAppContext *ctx = context;
    CFPropertyListRef plist = value;
    const CFTypeID t = CFGetTypeID (plist);

    if (ctx->ec != EC_OK) {
        return;
    }

    if (t != CFStringGetTypeID()) {
        char buf[80];
        snprintf (buf, sizeof (buf),
                  "Array element %lu is %s, not CFString",
                  ctx->count, typeString (plist));
        AL_clear (ctx->dest);
        AL_add (ctx->dest, buf);
        ctx->ec = EC_OTHER;
        return;
    }

    addString (plist, ctx->dest);
    ctx->count++;
}

static void dateApp (const void *value, void *context) {
    DateContext *ctx = context;
    CFPropertyListRef plist = value;
    const CFTypeID t = CFGetTypeID (plist);

    if (t == CFDateGetTypeID()) {
        const CFAbsoluteTime abt = CFDateGetAbsoluteTime (plist);
        const CFTimeInterval t1970 = abt + kCFAbsoluteTimeIntervalSince1970;
        if (! ctx->date->secondsValid) {
            MyDate_set_fractional (ctx->date, t1970);
        }
    } else if (*(ctx->errmsg) == NULL) {
        char buf[80];
        snprintf (buf, sizeof (buf),
                  "Array element %lu is %s, not CFDate",
                  ctx->count, typeString (plist));
        *(ctx->errmsg) = MY_STRDUP (buf);
    }

    ctx->count++;
}

static ErrorCode addArray (CFPropertyListRef plist, ArrayList *dest) {
    const CFTypeID t = CFGetTypeID (plist);

    if (t != CFArrayGetTypeID()) {
        char buf[80];
        snprintf (buf, sizeof (buf),
                  "Property list is %s, not CFArray", typeString (plist));
        AL_add (dest, buf);
        return EC_OTHER;
    }

    ArrAppContext ctx;
    ctx.dest = dest;
    ctx.count = 0;
    ctx.ec = EC_OK;

    const CFIndex count = CFArrayGetCount (plist);
    CFArrayApplyFunction (plist, CFRangeMake (0, count), arrApp, &ctx);
    return ctx.ec;
}

static ErrorCode dateArray (CFPropertyListRef plist,
                            MyDate *date,
                            char **errmsg) {
    const CFTypeID t = CFGetTypeID (plist);

    if (t != CFArrayGetTypeID()) {
        char buf[80];
        snprintf (buf, sizeof (buf),
                  "Property list is %s, not CFArray", typeString (plist));
        *errmsg = MY_STRDUP (buf);
        return EC_OTHER;
    }

    DateContext ctx;
    ctx.date = date;
    ctx.errmsg = errmsg;
    ctx.count = 0;

    const CFIndex count = CFArrayGetCount (plist);
    CFArrayApplyFunction (plist, CFRangeMake (0, count), dateApp, &ctx);
    return (*errmsg ? EC_OTHER : EC_OK);
}

ErrorCode props2list (const void *data, size_t length, ArrayList *dest) {
    CFDataRef d = CFDataCreate (NULL, data, length);
    CHECK_NULL (d);

    CFErrorRef err = NULL;
    CFPropertyListRef plist =
        CFPropertyListCreateWithData (NULL, d, kCFPropertyListImmutable,
                                      NULL, &err);
    CFRelease (d);

    if (plist == NULL) {
        CFStringRef msg = CFErrorCopyDescription (err);
        CHECK_NULL (msg);
        addString (msg, dest);
        CFRelease (msg);
        CFRelease (err);
        return EC_OTHER;
    }

    const ErrorCode ec = addArray (plist, dest);
    CFRelease (plist);
    return ec;
}

ErrorCode props2time (const void *data,
                      size_t length,
                      MyDate *date,
                      char **errmsg) {
    CFDataRef d = CFDataCreate (NULL, data, length);
    CHECK_NULL (d);

    CFErrorRef err = NULL;
    CFPropertyListRef plist =
        CFPropertyListCreateWithData (NULL, d, kCFPropertyListImmutable,
                                      NULL, &err);
    CFRelease (d);

    if (plist == NULL) {
        if (*errmsg == NULL) {
            CFStringRef msg = CFErrorCopyDescription (err);
            CHECK_NULL (msg);
            *errmsg = copyString (msg);
            CFRelease (msg);
        }
        CFRelease (err);
        return EC_OTHER;
    }

    const ErrorCode ec = dateArray (plist, date, errmsg);
    CFRelease (plist);
    return ec;
}

#endif  /* __APPLE__ */
