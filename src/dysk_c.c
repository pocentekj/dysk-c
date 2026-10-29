#include "dysk_c.h"

#include "dbg.h"
#include "deferred.h"
#include "strutil.h"

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statvfs.h>

#include <klib/kvec.h>

#define foreach(A) for (size_t iter = 0; iter < kv_size((A)); iter++)

int run(const prog_ops ops) {
    deferred(string_array_destroy) string_array *lines = string_array_new();
    check_ret(lines, EXIT_FAILURE, "Out of memory");

    check_ret(read_lines(ops.path, lines), EXIT_FAILURE, "Failed to read data");
    debug("%s: %zu lines", ops.path, string_array_len(lines));

    EntryTable entries;
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
    print_entries(entries, ops.human_readable, ops.display_format);

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

void print_entries(EntryTable entries, const bool hr, const dspl_format fmt) {
    switch (fmt) {

    case DSC_DSPL_JSON:
        printf("[");
        foreach (entries) {
            Entry *e = kv_A(entries, iter);
            Entry_print_json(e, hr);
            Entry_destroy(&e);
            if (iter + 1 < kv_size(entries)) printf(",");
        }
        printf("]\n");
        break;

    case DSC_DSPL_TABLE:
        // clang-format off
        printf("┌──────────────────┬─────────┬────────┬────────────────┬────────┐\n");
        printf("│ Filesystem       │ Type    │ Size   │ Use            │ Free   │\n");
        printf("├──────────────────┼─────────┼────────┼────────────────┼────────┤\n");
        // clang-format on

        foreach (entries) {
            Entry *e = kv_A(entries, iter);
            Entry_print_table(e);
            Entry_destroy(&e);
        }

        // clang-format off
        printf("└──────────────────┴─────────┴────────┴────────────────┴────────┘\n");
        // clang-format on

        break;
    }

    kv_destroy(entries);
}

string human_readable(const uint64_t bytes) {
    static const char *const units[] = {"B", "K", "M", "G", "T", "P", "E"};

    long double value = (long double)bytes;
    size_t unit = 0;
    const size_t last_unit = sizeof units / sizeof units[0] - 1;

    while (value >= 1000.0L && unit < last_unit) {
        value /= 1000.0L;
        ++unit;
    }

    // Avoid displaying a rounded-up "1000.0 KiB" instead of "1.0 MiB".
    if (unit < last_unit && value >= 999.95L) {
        value /= 1000.0L;
        ++unit;
    }

    int needed = unit == 0 ? snprintf(NULL, 0, "%" PRIu64 "%s", bytes, units[unit])
                           : snprintf(NULL, 0, "%.1Lf%s", value, units[unit]);

    if (needed < 0) return NULL;

    string result = malloc((size_t)needed + 1);
    if (result == NULL) return NULL;

    if (unit == 0)
        snprintf(result, (size_t)needed + 1, "%" PRIu64 "%s", bytes, units[unit]);
    else
        snprintf(result, (size_t)needed + 1, "%.1Lf%s", value, units[unit]);

    return result;
}

void print_progress_bar(const double prc) {
    if (prc < 0.0 || prc > 100.0) return;

    int v = (int)prc;
    if (prc - v >= 0.5) v += 1;

    static const int max_cols = 10;
    const int taken = v / max_cols;

    printf("%2d%% " BG_RED, v);
    for (int i = 0; i < taken; i++)
        putchar(' ');
    printf(BG_GREEN);
    for (int i = 0; i < max_cols - taken; i++)
        putchar(' ');
    printf(COLOR_RESET);
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
    e->avail = (uint64_t)vfs.f_bavail * (uint64_t)vfs.f_frsize;

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

void Entry_print_json(const Entry *e, const bool hr) {
    assert(e);

    if (hr) {
        deferred(free_string) string s_total = human_readable(e->total);
        deferred(free_string) string s_avail = human_readable(e->avail);
        check_nret(s_total && s_avail, "Out of memory");

        printf("{\"device\":\"%s\",\"mount_point\":\"%s\",\"file_system\":\"%s\","
               "\"total\":\"%s\",\"available\":\"%s\"}",
               e->dev, e->mp, e->fs, s_total, s_avail);
    } else {
        printf("{\"device\":\"%s\",\"mount_point\":\"%s\",\"file_system\":\"%s\","
               "\"total\":%" PRIu64 ",\"available\":%" PRIu64 "}",
               e->dev, e->mp, e->fs, e->total, e->avail);
    }
}

void Entry_print_table(const Entry *e) {
    const double used_prc = (double)(e->total - e->avail) / (double)e->total * 100.0;

    printf("│ %-16s │ %-7s │", e->dev, e->fs);

    deferred(free_string) string s_total = human_readable(e->total);
    deferred(free_string) string s_avail = human_readable(e->avail);
    check_nret(s_total && s_avail, "Out of memory");

    printf(COLOR_YELLOW " %6s " COLOR_RESET "│ ", s_total);
    print_progress_bar(used_prc);
    printf(" │" COLOR_GREEN " %6s " COLOR_RESET "│\n", s_avail);
}
