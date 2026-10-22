#include "strutil.h"

#include "dbg.h"
#include "deferred.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <klib/kvec.h>

string string_from_file(const string path, size_t *max_size) {
    assert(path);

    if (!path) return NULL;

    deferred(close_fp) FILE *fp = fopen(path, "r");
    check_ret(fp, NULL, "Failed to open '%s'", path);

    size_t cap = (max_size && *max_size < SIZE_MAX - 1) ? *max_size : SIZE_MAX - 1;

    check_ret(fseek(fp, 0L, SEEK_END) == 0, NULL, "fseek()");

    long file_size = ftell(fp);
    check_ret(file_size >= 0, NULL, "Invalid file size reported; aborting");
    if ((uintmax_t)file_size >= (uintmax_t)cap) {
        log_err("File '%s' is too large to process: %ld bytes", path, file_size);
        errno = EFBIG;
        return NULL;
    }

    check_ret(fseek(fp, 0L, SEEK_SET) == 0, NULL, "fseek()");

    string contents = malloc((size_t)file_size + 1);
    check_ret(contents, NULL, "Out of memory");

    size_t nbytes = fread(contents, 1, (size_t)file_size, fp);
    if (nbytes != (size_t)file_size) {
        log_err("Failed to read from '%s'", path);
        free_string(&contents);
        return NULL;
    }

    contents[file_size] = '\0';

    return contents;
}

string_array *string_array_new(void) {
    string_array *arr = calloc(1, sizeof *arr);
    check_ret(arr, NULL, "Out of memory");
    kv_init(arr->data);
    return arr;
}

void string_array_destroy(string_array **arr) {
    if (!arr || !*arr) return;

    for (size_t i = 0; i < kv_size((*arr)->data); i++) {
        free(kv_A((*arr)->data, i));
    }

    kv_destroy((*arr)->data);
    free(*arr);
    *arr = NULL;
}

bool read_lines(string path, string_array *out) {
    assert(path && out);

    deferred(close_fp) FILE *fp = fopen(path, "r");
    check_ret(fp, false, "Failed to open '%s'", path);

    deferred(free_string) string line = NULL;
    size_t max_size = 0;
    ssize_t nread = 0;

    while ((nread = getline(&line, &max_size, fp)) != -1) {
        if (nread > 0 && line[nread - 1] == '\n') line[--nread] = '\0';

        string tmp = strdup(line);
        check_ret(tmp, false, "Out of memory");

        kv_push(string, out->data, tmp);
    }

    return true;
}
