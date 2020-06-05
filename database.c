#include "whence.h"

#ifdef __APPLE__

#include <sqlite3.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define DB_FILENAME \
    "/Library/Preferences/com.apple.LaunchServices.QuarantineEventsV2"

#define QUERY \
    "SELECT * FROM LSQuarantineEvent WHERE LSQuarantineEventIdentifier == '"

typedef struct DBCtx {
    Attributes *dest;
    const char *uuid;
} DBCtx;

static char *get_dbname (void) {
    uid_t u = getuid ();
    errno = 0;
    struct passwd *pw = getpwuid (u);
    if (pw == NULL) {
        return NULL;
    }

    ArrayList al;
    AL_init (&al);
    AL_add (&al, pw->pw_dir);
    AL_add (&al, DB_FILENAME);
    char *dbname = AL_join (&al);
    AL_cleanup (&al);

    endpwent ();

    return dbname;
}

static void handleKey (const char *key, const char *value, Attributes *dest) {
    char **field = NULL;

    if (0 == strcmp (key, "LSQuarantineOriginURLString")) {
        field = &dest->referrer;
    } else if (0 == strcmp (key, "LSQuarantineDataURLString")) {
        field = &dest->url;
    }

    if (field != NULL && *field == NULL) {
        *field = strdup (value);
        CHECK_NULL (*field);
    }
}


static int callback (void *v, int nCols, char **values, char **names) {
    DBCtx *ctx = (DBCtx *) v;

    int i;
    for (i = 0; i < nCols; i++) {
        handleKey (names[i], values[i], ctx->dest);
    }

    return 0;
}

static ErrorCode run_query (Attributes *dest, const char *uuid, sqlite3 *sq) {
    char *errmsg = NULL;
    ErrorCode ec = EC_OK;
    DBCtx ctx;

    ctx.dest = dest;
    ctx.uuid = uuid;

    ArrayList al;
    AL_init (&al);
    AL_add (&al, QUERY);
    AL_add (&al, uuid);
    AL_add (&al, "'");
    char *query = AL_join (&al);
    AL_cleanup (&al);

    const int err = sqlite3_exec (sq, query, callback, &ctx, &errmsg);
    if (err != SQLITE_OK) {
        const char *msg = errmsg ? errmsg : "unknown";
        if (dest->error == NULL) {
            dest->error = strdup (msg);
            CHECK_NULL (dest->error);
        }
        ec = EC_OTHER;
        goto done;
    }

 done:
    sqlite3_free (errmsg);
    return ec;
}

ErrorCode lookup_uuid (Attributes *dest, const char *uuid) {
    char *dbname = get_dbname ();
    if (dbname == NULL) {
        int errnum = errno;
        if (dest->error == NULL && errnum != 0) {
            dest->error = strdup (strerror (errnum));
            CHECK_NULL (dest->error);
            return EC_OTHER;
        }
    }

    ErrorCode ec = EC_OK;
    sqlite3 *sq = NULL;
    const int err = sqlite3_open_v2 (dbname, &sq, SQLITE_OPEN_READONLY, NULL);
    CHECK_NULL (sq);
    if (err != SQLITE_OK) {
        if (dest->error == NULL) {
            dest->error = strdup (sqlite3_errmsg (sq));
            CHECK_NULL (dest->error);
        }
        ec = EC_OTHER;
        goto done;
    }

    ec = run_query (dest, uuid, sq);

 done:
    free (dbname);
    sqlite3_close (sq);
    return ec;
}

#endif  /* __APPLE__ */
