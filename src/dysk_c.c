#include "dysk_c.h"

#include "dbg.h"
#include "deferred.h"
#include "strutil.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <klib/kvec.h>

#define foreach(A) for (size_t iter = 0; iter < kv_size((A)); iter++)

int run(const string path) {
    deferred(string_array_destroy) string_array *lines = string_array_new();
    check_ret(lines, EXIT_FAILURE, "Out of memory");

    check_ret(read_lines(path, lines), EXIT_FAILURE, "Failed to read data");
    check_ret(string_array_len(lines) > 0, EXIT_FAILURE, "No entries");
    debug("%zu lines read.", string_array_len(lines));

    kvec_t(Entry *) entries;
    kv_init(entries);

    foreach (lines->data) {
        const string line = string_array_at(lines, iter);
        parse_result r = Entry_from_line(line);
        if (!r.ok) {
            if (r.status == PARSE_IGNORE) continue;
            log_err("Error processing line: %s", line);
            return EXIT_FAILURE;
        }

        kv_push(Entry *, entries, r.entry);
    }

    debug("%zu entries loaded", kv_size(entries));

    foreach (entries) Entry_destroy(&(kv_A(entries, iter)));
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
        r.status = PARSE_NOMEM;
        return r;
    }

    string_array *tokens = parse_line(line);
    if (!tokens) {
        r.status = PARSE_ERROR;
        goto error;
    }

    if (!starts_with(string_array_at(tokens, 0), "/dev")) {
        r.status = PARSE_IGNORE;
        goto error;
    }

    e->dev = strdup(string_array_at(tokens, 0));
    e->mp = strdup(string_array_at(tokens, 1));
    if (!e->dev || !e->mp) {
        r.status = PARSE_NOMEM;
        goto error;
    }
    r.entry = e;

    string_array_destroy(&tokens);
    r.status = PARSE_SUCCESS;
    r.ok = true;

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

    free(*e);
    *e = NULL;
}
