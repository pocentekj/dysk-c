#pragma once

#include "strutil.h"
#include <stdbool.h>
#include <stdint.h>

#include <klib/kvec.h>

typedef struct {
    string dev;     // device
    string mp;      // mount point
    string fs;      // file system type
    uint64_t total; // total space (in bytes)
    uint64_t avail; // available space (in bytes)
} Entry;

typedef kvec_t(Entry *) EntryTable;

typedef enum {
    PARSE_SUCCESS,
    PARSE_IGNORE, // by default, everything not in /dev is ignored
    PARSE_FORMAT, // invalid file (line) format
    PARSE_ENOMEM, // out of memory
    PARSE_ERRVFS, // statvfs error
} p_status;

typedef enum {
    DSC_DSPL_JSON,
    DSC_DSPL_TABLE,
} dspl_format;

typedef struct {
    bool ok;
    p_status status;
    Entry *entry; // NULL if ok = false
} parse_result;

parse_result Entry_from_line(const string line);
void Entry_destroy(Entry **e);
void Entry_print_json(const Entry *e, const bool hr);
void Entry_print_table(const Entry *e, const bool hr);

/**
 * Print all entries in selected format. This will consume and destroy `entries`.
 */
void print_entries(EntryTable entries, const bool hr, const dspl_format fmt);

/**
 * Try to split line into tokens. Returns NULL on failure.
 */
string_array *parse_line(const string line);

/**
 * Convert raw bytes into human-readable format (e.g. 2.5 GB).
 * Returns string with converted value or NULL on failure.
 * Note that caller must free returned string.
 */
string human_readable(uint64_t bytes);

typedef struct {
    string path;
    dspl_format display_format;
    bool human_readable;
} prog_ops;

int run(const prog_ops ops);
