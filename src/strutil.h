#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include <klib/kvec.h>

typedef char *string;

/**
 * This function takes pointer to string - it's useful for `deferred` callback.
 */
static inline void free_string(string *s) {
    if (!s || !*s) return;
    free(*s);
    *s = NULL;
}

/**
 * Read entire file on `path` into string.
 *
 * If `max_size` is provided, it will be used as the upper size limits for files
 * that should be loaded. This function will reject bigger files and return NULL.
 * If `max_size` is NULL, the upper limit will be set to SIZE_MAX -1.
 *
 * Returns pointer to string on success, NULL otherwise. Note, that the pointer
 * returned must be released by calling `free` (or `free_string(&ptr)`).
 */
string string_from_file(const string path, size_t *max_size);

typedef struct {
    kvec_t(string) data;
} string_array;

string_array *string_array_new(void);

/**
 * This function takes pointer to pointer as an argument - to be used as
 * a callback for `deferred` macro.
 */
void string_array_destroy(string_array **arr);

static inline size_t string_array_len(const string_array *arr) {
    assert(arr);
    return arr ? kv_size(arr->data) : 0;
}

/**
 * Read entire file on `path` line by line, and store lines in `out` string_array.
 * Returns `true` on success, `false` on error. `errno` may be set by underlying
 * functions, such as `fopen`.
 */
bool read_lines(string path, string_array *out);
