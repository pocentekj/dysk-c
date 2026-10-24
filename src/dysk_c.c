#include "dysk_c.h"

#include "dbg.h"
#include "deferred.h"
#include "strutil.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

#include <klib/kvec.h>

#define foreach(A) for (size_t iter = 0; iter < kv_size((A)); iter++)

int run(const string path) {
    deferred(string_array_destroy) string_array *lines = string_array_new();
    check_ret(lines, EXIT_FAILURE, "Out of memory");

    check_ret(read_lines(path, lines), EXIT_FAILURE, "Failed to read data");
    debug("%zu lines read.", string_array_len(lines));

    kvec_t(Entry *) entries;
    kv_init(entries);

    foreach (lines->data) {
        const string line = string_array_at(lines, iter);
        parse_result r = Entry_from_line(line);

        if (!r.ok) {
            if (r.status == PARSE_IGNORE) continue;

            switch (r.status) {
            case PARSE_FORMAT:
                log_err("Invalid format: %s", line);
                break;
            case PARSE_ENOMEM:
                log_err("Out of memory");
                break;
            case PARSE_ERRVFS:
                log_err("Cannot fetch statvfs: %s", line);
                break;
            default:
                log_err("Error processing line: %s", line);
                break;
            }

            return EXIT_FAILURE;
        }

        kv_push(Entry *, entries, r.entry);
    }

    debug("%zu entries loaded", kv_size(entries));

    foreach (entries) {
        Entry_destroy(&(kv_A(entries, iter)));
    }
    kv_destroy(entries);

    return EXIT_SUCCESS;
}

string_array *parse_line(const string line) {
    assert(line);

    if (!line) return NULL;

    string_array *tokens = string_array_new();
    if (!tokens) return NULL;

    string saveptr = NULL;
    string delim = " ";
    string token = strtok_r(line, delim, &saveptr);
    while (token) {
        string tmp = strdup(token);
        check_mem(tmp);
        kv_push(string, tokens->data, tmp);
        token = strtok_r(NULL, delim, &saveptr);
    }

    check(string_array_len(tokens) == 6, "Invalid line: %s", line);

    return tokens;

error:
    string_array_destroy(&tokens);
    return NULL;
}

parse_result Entry_from_line(const string line) {
    parse_result r = {0};

    Entry *e = calloc(1, sizeof *e);
    if (!e) {
        r.status = PARSE_ENOMEM;
        return r;
    }

    string_array *tokens = parse_line(line);
    if (!tokens) {
        r.status = PARSE_FORMAT;
        goto error;
    }

    // Filter out only physical devices - note that it's not really reliable
    // method, but it works for me now, so I'll just leave it.
    if (!starts_with(string_array_at(tokens, 0), "/dev")) {
        r.status = PARSE_IGNORE;
        goto error;
    }

    e->dev = strdup(string_array_at(tokens, 0));
    e->mp = strdup(string_array_at(tokens, 1));
    e->fs = strdup(string_array_at(tokens, 2));
    if (!e->dev || !e->mp || !e->fs) {
        r.status = PARSE_ENOMEM;
        goto error;
    }

    struct statvfs vfs;
    if (statvfs(e->mp, &vfs) == -1) {
        r.status = PARSE_ERRVFS;
        goto error;
    }

    e->total = (uint64_t)vfs.f_blocks * (uint64_t)vfs.f_frsize;
    e->avail = (uint64_t)vfs.f_bfree * (uint64_t)vfs.f_frsize;

    r.entry = e;
    r.status = PARSE_SUCCESS;
    r.ok = true;

    string_array_destroy(&tokens);

    return r;

error:
    string_array_destroy(&tokens);
    Entry_destroy(&e);
    return r;
}

void Entry_destroy(Entry **e) {
    if (!e || !*e) return;

    if ((*e)->dev) free((*e)->dev);
    if ((*e)->mp) free((*e)->mp);
    if ((*e)->fs) free((*e)->fs);

    free(*e);
    *e = NULL;
}
