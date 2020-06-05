#include "whence.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <string.h>
#include <stdlib.h>

#define SUBKEY \
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\"

static char *get_reg (HKEY key, const char *subkey) {
    const char *value = "DisplayName";
    DWORD len = 0;

    LSTATUS st = RegGetValue (key, subkey, value,
                              RRF_RT_REG_SZ, NULL, NULL, &len);
    if (st != ERROR_SUCCESS || len == 0) {
        return NULL;
    }

    char *result = malloc (len);
    CHECK_NULL (result);
    st = RegGetValue (key, subkey, value,
                      RRF_RT_REG_SZ, NULL, result, &len);
    if (st != ERROR_SUCCESS) {
        free (result);
        return NULL;
    }

    return result;
}

static char *lookupZoneName (const char *zoneNumber) {
    ArrayList al;

    AL_init (&al);
    AL_add (&al, SUBKEY);
    AL_add (&al, zoneNumber);
    char *subkey = AL_join (&al);
    AL_cleanup (&al);

    char *name = get_reg (HKEY_CURRENT_USER, subkey);
    if (name == NULL) {
        name = get_reg (HKEY_LOCAL_MACHINE, subkey);
    }

    free (subkey);
    return name;
}

const char *getZoneName (const char *zoneNumber, ZoneCache *zc) {
    size_t i;

    /* linear search for cached zone */
    for (i = 0; i < zc->keys.size; i++) {
        if (0 == strcmp (zoneNumber, zc->keys.strings[i])) {
            return (zc->values.strings[i]);
        }
    }

    if (zc->keys.size >= 100) {
        /* This is incredibly unlikely to happen, since typically there are
         * about five zones or so.  But if we were for some reason caching
         * a large number of zones, clear it out so we don't get O(n^2)
         * behavior with linear search. */
        AL_clear (&zc->keys);
        AL_clear (&zc->values);
    }

    char *name = lookupZoneName (zoneNumber);
    AL_add (&zc->keys, zoneNumber);
    if (name) {
        AL_add_nocopy (&zc->keys, name);
    } else {
        /* If no name found, cache the zone number as the name,
         * so we don't keep looking it up every time. */
        AL_add (&zc->keys, zoneNumber);
    }

    return (name ? name : zoneNumber);
}

void Cache_init (ZoneCache *cache) {
    AL_init (&cache->keys);
    AL_init (&cache->values);
}

void Cache_cleanup (ZoneCache *cache) {
    AL_cleanup (&cache->keys);
    AL_cleanup (&cache->values);
}

#endif  /* _WIN32 */
