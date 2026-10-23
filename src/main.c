#include "dbg.h"
#include "deferred.h"
#include "strutil.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

const string DEFAULT_FILE_PATH = "/proc/mounts";

int main(int argc, char *argv[]) {
    if (setlocale(LC_ALL, "") == NULL) log_warn("setlocale() failed");

    const string path = (argc == 2) ? argv[1] : DEFAULT_FILE_PATH;
    deferred(string_array_destroy) string_array *lines = string_array_new();
    check_mem(lines);
    check(read_lines(path, lines), "Failed to read data");
    debug("Loaded %zu entries.", string_array_len(lines));

    check(fflush(stdout) == 0, "fflush()");
    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}
