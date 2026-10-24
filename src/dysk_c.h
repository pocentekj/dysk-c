#pragma once

#include "strutil.h"

#include <stdbool.h>

typedef struct {
    string dev;
    string mp;
} Entry;

typedef enum {
    PARSE_SUCCESS,
    PARSE_IGNORE,
    PARSE_ERROR,
    PARSE_NOMEM,
} p_status;

typedef struct {
    bool ok;
    p_status status;
    Entry *entry;
} parse_result;

parse_result Entry_from_line(const string line);
void Entry_destroy(Entry **e);

string_array *parse_line(const string line);
int run(const string path);
