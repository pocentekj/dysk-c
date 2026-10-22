#include "dbg.h"
#include "deferred.h"
#include "strutil.h"

#include <locale.h>
#include <stdlib.h>

const string DEFAULT_FILE_PATH = "/proc/mounts";

int main(int argc, char *argv[]) {
    if (!setlocale(LC_ALL, "")) log_warn("setlocale() failed");

    string path = (argc == 2) ? argv[1] : DEFAULT_FILE_PATH;

    deferred(string_array_destroy) string_array *lines = string_array_new();
    check_ret(lines, EXIT_FAILURE, "Out of memory");
    check_ret(read_lines(path, lines), EXIT_FAILURE, "Failed to read data");

    debug("Loaded %zu entries.", string_array_len(lines));

    return fflush(stdout) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
