#include "dysk_c.h"

#include "dbg.h"
#include "deferred.h"
#include "strutil.h"

#include <stdlib.h>

int run(const string path) {
    deferred(string_array_destroy) string_array *lines = string_array_new();
    check_mem(lines);
    check(read_lines(path, lines), "Failed to read data");
    debug("Loaded %zu entries.", string_array_len(lines));
    return EXIT_SUCCESS;
error:
    return EXIT_FAILURE;
}
