#pragma once

#include "strutil.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    string dev;     // device
    string mp;      // mount point
    string fs;      // file system type
    uint64_t total; // total space (in bytes)
    uint64_t avail; // available space (in bytes)
} Entry;

typedef enum {
    PARSE_SUCCESS,
    PARSE_IGNORE, // by default, everything not in /dev is ignored
    PARSE_FORMAT, // invalid file (line) format
    PARSE_ENOMEM, // out of memory
    PARSE_ERRVFS, // statvfs error
} p_status;

typedef struct {
    bool ok;
    p_status status;
    Entry *entry; // NULL if ok = false
} parse_result;

parse_result Entry_from_line(const string line);
void Entry_destroy(Entry **e);
void Entry_print_json(Entry *e);

/**
 * Try to split line into tokens. Returns NULL on failure.
 */
string_array *parse_line(const string line);

int run(const string path);
