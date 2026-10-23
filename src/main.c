#include "dbg.h"
#include "dysk_c.h"
#include "strutil.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

const string DEFAULT_FILE_PATH = "/proc/mounts";

int main(int argc, char *argv[]) {
    if (setlocale(LC_ALL, "") == NULL) log_warn("setlocale() failed");

    const string path = (argc == 2) ? argv[1] : DEFAULT_FILE_PATH;
    int rc = run(path);
    check(fflush(stdout) == 0, "fflush()");

    return rc;
error:
    return EXIT_FAILURE;
}
