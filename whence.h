#ifndef WHENCE_H
#define WHENCE_H

#include <stddef.h>
#include <time.h>
#include <stdbool.h>

#define CMD_NAME "whence"

typedef enum ErrorCode {
    EC_OK = 0,
    EC_NOATTR = 1,
    EC_NOFILE = 2,
    EC_OTHER = 3,
    EC_CMDLINE = 4,
    EC_MEM = 5
} ErrorCode;

typedef struct ArrayList {
    char **strings;
    size_t size;
    size_t capacity;
} ArrayList;

typedef struct Attributes {
    char *url;
    char *referrer;
    char *from;
    char *subject;
    char *message_id;
    char *application;
    time_t date;
    char *zone;
    char *error;
} Attributes;

typedef enum AttrStyle {
    AS_HUMAN,
    AS_HUMAN_COLOR,
    AS_JSON_NOTLAST,
    AS_JSON_LAST
} AttrStyle;

typedef struct DatabaseConnection {
    void *db;
    bool triedOpening;
} DatabaseConnection;

typedef struct ZoneCache {
    ArrayList keys;
    ArrayList values;
} ZoneCache;

#ifdef __APPLE__
typedef DatabaseConnection Cache;
#elif defined (_WIN32)
typedef ZoneCache Cache;
#else
typedef int Cache;              /* dummy */
#endif

#define CHECK_NULL(x)                                           \
    do { if ((x) == NULL) oom (__FILE__, __LINE__); } while (0)

#define MY_STRDUP(x) my_strdup ((x), __FILE__, __LINE__)

/* getattr.c */
ErrorCode getAttribute (const char *fname,
                        const char *attr,
                        char **result,
                        size_t *length);

/* util.c */
void oom (const char *file, long line);
ErrorCode combineErrors (ErrorCode ec1, ErrorCode ec2);
char *my_strdup (const char *s, const char *file, long line);

/* array-list.c */
void AL_init (ArrayList *al);
void AL_add (ArrayList *al, const char *str);
void AL_add_nocopy (ArrayList *al, char *str);
char *AL_join (const ArrayList *al);
void AL_clear (ArrayList *al);
void AL_cleanup (ArrayList *al);

/* props.c */
ErrorCode props2list (const void *data, size_t length, ArrayList *dest);

/* split.c */
void split (const char *str, char sep, ArrayList *dest);

/* attributes.c */
void Attr_init (Attributes *attrs);
void Attr_print (const Attributes *attrs, const char *fname, AttrStyle style);
void Attr_cleanup (Attributes *attrs);

/* linux.c, macos.c, or windows.c */
ErrorCode getAttributes (const char *fname,
                         Attributes *dest,
                         Cache *cache);

/* linux.c, database.c, or registry.c */
void Cache_init (Cache *cache);
void Cache_cleanup (Cache *cache);

/* color.c */
bool enableColorEscapes (int fd);

/* database.c */
ErrorCode lookup_uuid (Attributes *dest,
                       const char *uuid,
                       DatabaseConnection *conn);

/* registry.c */
const char *getZoneName (const char *zoneNumber, ZoneCache *zc);

#endif  /* WHENCE_H */
