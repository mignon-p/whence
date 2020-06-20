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

#ifndef WHENCE_H
#define WHENCE_H

#include <stddef.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#include <wchar.h>
#endif

/* CMD_NAME is used in various error messages.  CMD_VERSION is used by
 * --version option.  CMD_VERSION is also extracted and used by
 * regenerate-manpage.sh script, to put correct version on man page.
 *
 * We follow the convention that if the rightmost number in
 * CMD_VERSION is even, this is a release version, and if it is odd,
 * this is a development version.
 */
#define CMD_NAME "whence"
#define CMD_VERSION "0.9.3"

/* Type for UTF-16 code units.  On Windows, wchar_t is 16 bits, so
 * use it for greater interoperability with Windows APIs that take
 * UTF-16 strings.  On UNIX, wchar_t is 32 bits, so use uint16_t
 * instead.
 */
#ifdef _WIN32
typedef wchar_t utf16;
#else
typedef uint16_t utf16;
#endif

/* Constants used as "color" argument to setColor().  These are just
 * the numbers used in the ANSI "\e[Xm" escape codes, where X is
 * the color number.
 */
#define COLOR_RED     91
#define COLOR_GREEN   92
#define COLOR_MAGENTA 95
#define COLOR_OFF      0

/* Error codes are used internally as return values from functions,
 * and also externally as the exit code from the "whence" command.
 * EC_MEM is only used as an exit code as is never returned from
 * a function, because out-of-memory errors are treated as fatal,
 * and terminate the program immediately.
 */
typedef enum ErrorCode {
    EC_OK = 0,                  /* no error */
    EC_NOATTR = 1,              /* attribute does not exist */
    EC_NOFILE = 2,              /* file does not exist */
    EC_OTHER = 3,               /* some other error */
    EC_CMDLINE = 4,             /* command-line parsing error */
    EC_MEM = 5                  /* out of memory */
} ErrorCode;

/* A list of strings.  Each string is malloced and must be freed.
 * The strings array is also malloced, and is realloced as necessary
 * as the array expands.
 */
typedef struct ArrayList {
    char **strings;             /* array of malloced strings */
    size_t size;                /* number of valid, malloced strings */
    size_t capacity;            /* capacity of strings array */
} ArrayList;

/* A date and time.  Keeps track of UNIX time in seconds, and also
 * optionally the number of milliseconds past the second. */
typedef struct MyDate {
    time_t seconds;             /* UNIX time */
    uint16_t milliseconds;      /* optional, milliseconds past the second */
    bool secondsValid;          /* is this MyDate valid at all? */
    bool millisValid;           /* do we have millisecond resolution? */
} MyDate;

/* Attributes of a file.  Each string is malloced and must be freed.
 * Each field is NULL if that attribute is not present on the file.
 * "error" is non-NULL if an error occurred, and contains the error
 * message.
 */
typedef struct Attributes {
    char *url;
    char *referrer;
    char *from;
    char *subject;
    char *message_id;
    char *application;
    MyDate date;
    char *zone;
    char *error;
} Attributes;

/* Style for printing attributes, passed to Attr_print(). */
typedef enum AttrStyle {
    AS_HUMAN,
    AS_HUMAN_COLOR,
    AS_JSON_NOTLAST,
    AS_JSON_LAST
} AttrStyle;

/* Only used on MacOS.  Connection to a SQLite3 database.
 * "db" is actually a "sqlite3 *", but we cast it to void
 * to avoid having to include sqlite3.h from whence.h.
 */
typedef struct DatabaseConnection {
    void *db;
    bool triedOpening;
} DatabaseConnection;

/* Only used on Windows.  The dual ArrayLists are used as a primitive
 * sort of map.  (The invariant is that keys->size == values->size.)
 * This structure caches the zone names looked up from the registry.
 * (I do not know whether registry accesses are expensive enough to
 * make caching worthwhile or not.)
 */
typedef struct ZoneCache {
    ArrayList keys;             /* zone numbers (as strings) */
    ArrayList values;           /* zone names */
} ZoneCache;

/* The Cache type is used to store information between calls to
 * getAttributes().  The information stored is platform-specific.
 */
#ifdef __APPLE__
typedef DatabaseConnection Cache;
#elif defined (_WIN32)
typedef ZoneCache Cache;
#else
typedef int Cache;              /* dummy */
#endif

/* Information about whether a file handle is a terminal.
 *
 * "is_terminal" is basically just the result of isatty().
 *
 * "supports_color" is only true if "is_terminal" is true, and only
 * if the terminal is believed to support ANSI color escapes.
 *
 * On Windows, the terminal is believed to support color if it is
 * possible to set the ENABLE_VIRTUAL_TERMINAL_PROCESSING flag on
 * the console, or if that flag is already set.  (That flag is
 * only supported on recent versions of Windows 10, so older
 * versions of Windows will not use color.)
 *
 * On UNIX, the terminal is believed to support color if the
 * TERM environment variable contains the substring "color", or
 * if the TERM environment variable is equal to one of a hardcoded
 * list of terminal names that are known to support color.
 */
typedef struct Terminal {
    bool is_terminal;
    bool supports_color;
} Terminal;

/* Use after every malloc to die instantly if NULL is returned. */
#define CHECK_NULL(x)                                           \
    do { if ((x) == NULL) oom (__FILE__, __LINE__); } while (0)

/* strdup() is used so frequently that MY_STRDUP() combines
 * strdup() with CHECK_NULL().
 */
#define MY_STRDUP(x) my_strdup ((x), __FILE__, __LINE__)

/* getattr.c or windows.c ------------------------------------------------ */

/* Get the attribute "attr" from the file named "fname".  New memory is
 * allocated and written to "*result".  The length of the result is written
 * to "*length".
 *
 * If the return value is EC_OK, then the result is the attribute value,
 * which may be a string, or may be binary.  If the return value is not
 * EC_OK, then the result is a UTF-8 string which further describes the
 * error.  Either way, the result must be freed by the caller.
 *
 * A NUL byte is stored after the result, to make it easier to deal
 * with if it is a string.  The NUL byte is not considered part of the
 * attribute, and is not counted in the length.
 */
ErrorCode getAttribute (const char *fname,
                        const char *attr,
                        char **result,
                        size_t *length);

/* Returns a malloced string which must be freed by the caller.
 * On UNIX, "fname" is returned unchanged and "drives" is unused, so
 * fixFilename() is basically a glorified strdup().
 *
 * On Windows, the returned filename may be modified to fix the two
 * problems described below.  "drives" should be initialzed to -1 by
 * the caller before the first call to fixFilename(), and should be
 * preserved between calls.  It is used to cache the result of
 * GetLogicalDrives().  (Again, I don't know if GetLogicalDrives()
 * is expensive enough to be worth caching or not.)
 *
 * Problem #1:
 *
 * So, I've noticed a weird problem I don't really understand.
 * Normally, on Windows, absolute filenames get passed like this:
 *   C:/foo/bar/User Guide.pdf
 *
 * However, if the filename contains the single quote character "'",
 * it instead gets passed like this:
 *   /c/foo/bar/User's Guide.pdf
 *
 * fopen() knows how to open the "C:/" form, but doesn't seem to be
 * able to open the "/c/" form.
 *
 * This function rewrites "/c/" to "C:/", only if:
 *   - The original filename is not accessible
 *   - The drive letter exists
 *   - The file is accessible under the new name
 *
 * Problem #2:
 *
 * If filename is a single letter, such as "c", then when we append
 * the stream name, it looks like "c:Zone.Identifier", which would be
 * interpreted as a file "Zone.Identifier" in the current directory of
 * drive C.  Therefore, in the case of single letter filenames, prepend
 * "./" so it doesn't look like a drive letter.
 */
char *fixFilename (const char *fname, int32_t *drives);

/* util.c ---------------------------------------------------------------- */

/* Print an error message referencing the given file and line, and
 * exit the program with code EC_MEM.  This is called by the CHECK_NULL()
 * macro.
 */
void oom (const char *file, long line);

/* Combines two error codes.  Generally the higher-numbered error is
 * given preference, except that EC_OK is preferred over EC_NOATTR.
 */
ErrorCode combineErrors (ErrorCode ec1, ErrorCode ec2);

/* Duplicate a string by calling strdup(), and then die if
 * strdup() returns NULL.  Used by MY_STRDUP() macro.
 */
char *my_strdup (const char *s, const char *file, long line);

/* Format the given arguments, printf-style, and print them to stderr.
 * If stderrTerminal.supports_color is true, send ANSI escapes so that
 * the message is printed in red.  err_printf() prints a newline after
 * printing the message, so the message itself should not end with a
 * newline.
 */
void err_printf (const char *fmt, ...)
#ifdef __GNUC__
    __attribute__ ((format (printf, 1, 2)))
#endif
    ;

/* Starting at "s", count the number of bytes with the hi bit set.
 * This represents one or more non-ASCII Unicode characters encoded
 * in UTF-8.  Converts these character(s) to UTF-16, and prints one
 * or more JSON-style "\uXXXX" escape sequences to stdout, representing
 * these UTF-16 code points.
 *
 * Returns the number of bytes processed.  (i. e. advancing s by the
 * return value will make s point at the the next byte that does not
 * have the hi bit set.)
 */
size_t print_escaped_unicode (const char *s);

/* Possibly prints an ANSI escape code to the stream "f", which will
 * set the text color to "color", which is one of the "COLOR_*" defines
 * from earlier in this header file.
 *
 * If "useColor" is true, prints the escape code.  If "useColor" is
 * false, does nothing.
 */
void setColor (FILE *f, bool useColor, int color);

/* Returns true if the environment variable NO_COLOR is set to a
 * non-empty value.  This is used by detectConsole().
 * See https://no-color.org/
 */
bool envNoColor (void);

/* array-list.c ---------------------------------------------------------- */

/* Initializes an ArrayList structure, such that it contains the empty list. */
void AL_init (ArrayList *al);

/* Copies "str" and appends it to the given ArrayList. */
void AL_add (ArrayList *al, const char *str);

/* Appends "str" to the given ArrayList without copying.  Ownership of "str"
 * is transferred to the ArrayList, and will be freed by AL_cleanup(). */
void AL_add_nocopy (ArrayList *al, char *str);

/* Concatenates all the elements of the ArrayList into one big string and
 * returns it.  (This is a newly allocated string which must be freed
 * by the caller.) */
char *AL_join (const ArrayList *al);

/* Frees all the strings referenced by the ArrayList, and sets the size
 * (but not the capacity) of the ArrayList to 0. */
void AL_clear (ArrayList *al);

/* Frees all memory used by the ArrayList.  This is like AL_clear(), but
 * also frees the "strings" array itself. */
void AL_cleanup (ArrayList *al);

/* props.c --------------------------------------------------------------- */

/* MacOS only.  Given a binary property list specified by "data" and
 * "length", interprets it as an array (CFArray) of strings
 * (CFString), and adds the strings to the ArrayList "dest", which
 * must have been already initialized by the caller.  If the return
 * code is not EC_OK, then "dest" will contain exactly one string,
 * which is an error message.  All of the strings are encoded in
 * UTF-8.
 */
ErrorCode props2list (const void *data, size_t length, ArrayList *dest);

/* MacOS only.  Given a binary property list specified by "data" and
 * "length", interprets it as an array (CFArray) of dates (CFDate),
 * which ought to just contain a single date.  On success, converts
 * the date to a MyDate with millisecond accuary, write it to "*date",
 * and returns EC_OK.
 *
 * On error, allocates a new string containing an error message,
 * writes it to "*errmsg", and returns a code other than EC_OK.
 * The error message must be freed by the caller.
 */
ErrorCode props2time (const void *data,
                      size_t length,
                      MyDate *date,
                      char **errmsg);

/* split.c --------------------------------------------------------------- */

/* Splits "str" into multiple substrings, separated by "sep".
 * Adds the resulting strings to "dest", which must have been
 * already initialized by the caller.
 */
void split (const char *str, char sep, ArrayList *dest);

/* attributes.c ---------------------------------------------------------- */

/* Initializes an Attributes structure by setting all the string
 * attributes to NULL, and marking the date as invalid.
 */
void Attr_init (Attributes *attrs);

/* Print the given Attributes structure in the given style.
 * "fname" is the name of the file that the attributes belong to.
 * For JSON styles, prints everything to stdout.
 * For "human" styles, prints error messages to stderr, and everything
 * else to stdout.
 */
void Attr_print (const Attributes *attrs, const char *fname, AttrStyle style);

/* Frees all of the strings contained in the Attributes structure. */
void Attr_cleanup (Attributes *attrs);

/* xdg.c, macos.c, or windows.c ------------------------------------------ */

/* Gets the attributes of the file named "fname", and stores them in
 * "*dest".  "cache" is used to keep track of things between calls.
 * "cache" should have been initialized with Cache_init() before the
 * first call to getAttributes(), and should be cleaned up with
 * Cache_cleanup() after the last call to getAttributes().
 */
ErrorCode getAttributes (const char *fname,
                         Attributes *dest,
                         Cache *cache);

/* xdg.c (only on Mac OS) ------------------------------------------------ */

/* Only on MacOS.  A version of getAttributes() which checks for the
 * XDG attribute names, rather than the MacOS attribute names.  This is
 * because curl and wget use XDG attribute names, even on MacOS.
 * This function is called by getAttributes(), so getAttributes() on
 * MacOS will get both the MacOS attributes and the XDG attributes.
 */
ErrorCode getAttributes_xdg (const char *fname,
                             Attributes *dest);

/* xdg.c, database.c, or registry.c -------------------------------------- */

/* Initializes a Cache structure. */
void Cache_init (Cache *cache);

/* Frees all of the resources referenced by a Cache structure. */
void Cache_cleanup (Cache *cache);

/* term-unix.c or term-win32.c ------------------------------------------- */

/* Information about whether stdout is a terminal. */
extern Terminal stdoutTerminal;

/* Information about whether stderr is a terminal. */
extern Terminal stderrTerminal;

/* Sets the stdoutTerminal and stderrTerminal variables.  On Windows,
 * if a console is detected, sets the ENABLE_VIRTUAL_TERMINAL_PROCESSING
 * flag on the console, so that ANSI color escapes will be processed.
 * Uses atexit() to arrange for the original console mode to be restored
 * on exit.
 */
void detectConsole (void);

/* Writes a UTF-8 encoded string to the specified file handle.
 * On UNIX, this is the same as fputs().
 * On Windows, this is the same as fputs() is the file handle does
 * not represent a console.
 * If the file handle is a console on Windows, converts the UTF-8
 * to UTF-16 and prints it using WriteConsoleW().
 */
void writeUTF8 (FILE *f, const char *s);

/* database.c ------------------------------------------------------------ */

/* MacOS only.  Looks up the given UUID (which should have been
 * obtained from the fourth field of the "com.apple.quarantine"
 * attribute) in the SQLite3 database
 * "~/Library/Preferences/com.apple.LaunchServices.QuarantineEventsV2".
 * If information about the URL and Referrer is found in the database,
 * set these fields in "*dest".
 * "conn" should have been initialized with Cache_init(), and should be
 * cleaned up with Cache_cleanup().
 */
ErrorCode lookup_uuid (Attributes *dest,
                       const char *uuid,
                       DatabaseConnection *conn);

/* MacOS only.  Returns a newly allocated string (which must be freed
 * by the caller) containing the version number of the SQLite3 library
 * and headers.  This is used by the "--version" command-line option.
 */
char *get_sqlite_version (void);

/* registry.c ------------------------------------------------------------ */

/* Windows only.  Look up the given zone number (in string form) in
 * the registry to determine the DisplayName of the zone.  Returns the
 * DisplayName of the zone.
 *
 * This function always "succeeds", in that it just returns the
 * zone number if it cannot determine the DisplayName of the zone.
 *
 * The returned string is owned by the ZoneCache, and should NOT be
 * freed by the caller.
 *
 * "zc" should have been initialized with Cache_init(), and should be
 * cleaned up with Cache_cleanup().
 */
const char *getZoneName (const char *zoneNumber, ZoneCache *zc);

/* date.c ---------------------------------------------------------------- */

/* Clears out a MyDate structure, so that it is marked as not containing
 * a valid date. */
void MyDate_clear (MyDate *date);

/* Sets "*date" to an integer number of seconds.  "t" is a UNIX time. */
void MyDate_set_integer (MyDate *date, time_t t);

/* Sets "*date" to a fractional number of seconds.  "t" is the number
 * of seconds since the UNIX epoch, as a floating point value. */
void MyDate_set_fractional (MyDate *date, double t);

/* Returns a newly allocated string representing the date in a
 * human-readable format, using the current locale.  Only uses
 * integer second resolution, even if the date has millisecond
 * resolution.
 */
char *MyDate_format_human (const MyDate *date);

/* Returns a newly allocated string representing the date in
 * ISO 8601/RFC 3339 format.  Specifically,
 * "yyyy-mm-ddThh:mm:ss.sssZ" if the date has millisecond resolution, or
 * "yyyy-mm-ddThh:mm:ssZ" if it does not.
 */
char *MyDate_format_iso8601 (const MyDate *date);

/* utf-iconv.c or utf-win32.c -------------------------------------------- */

/* Converts UTF-8 to UTF-16, or UTF-16 to UTF-8.  Returns a newly
 * allocated string that must be freed by the caller.  If an error
 * occurs (such as malformed UTF-8 or UTF-16), returns NULL.  (Like all
 * functions in "whence", these functions terminate the program
 * immediately if out of memory, so a NULL return value does not
 * represent an out-of-memory condition, just some other error.
 *
 * The *_len variants take a string and length instead of a NUL-terminated
 * string.
 *
 * The *_nofail variants terminate the program if an error occurs, and
 * never return NULL.
 */

utf16 *utf8to16 (const char *s);
char *utf16to8 (const utf16 *s);
utf16 *utf8to16_len (const char *s, size_t len);
char *utf16to8_len (const utf16 *s, size_t len);
utf16 *utf8to16_nofail (const char *s);
char *utf16to8_nofail (const utf16 *s);

/* win-err.c ------------------------------------------------------------- */

/* Converts a Windows error code, such as that returned by GetLastError(),
 * into a string using FormatMessageW().  Converts the error message
 * to UTF-8 and returns it in a newly allocated string, which must be
 * freed by the caller.
 */
char *getErrorString (uint32_t lastErr);

#endif  /* WHENCE_H */
