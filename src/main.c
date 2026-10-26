#include "dbg.h"
#include "dysk_c.h"
#include "strutil.h"

#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

const string DEFAULT_FILE_PATH = "/proc/mounts";

int main(int argc, char *argv[]) {
    if (setlocale(LC_ALL, "") == NULL) log_warn("setlocale() failed");

    static const struct option options[] = {
        {"human-readable", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0},
    };

    prog_ops ops = {
        .path = DEFAULT_FILE_PATH,
        .human_readable = false,
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "h", options, NULL)) != -1) {
        switch (opt) {
        case 'h':
            ops.human_readable = true;
            break;
        default:
            fprintf(stderr, "Usage: dsc -[h] [PATH]\n");
            break;
        }
    }

    if (optind < argc) ops.path = argv[optind];

    debug("human_readable = %s", ops.human_readable ? "Yes" : "No");
    debug("path = %s", ops.path);

    int rc = run(ops);
    check_ret(fflush(stdout) == 0, EXIT_FAILURE, "fflush()");

    return rc;
}
