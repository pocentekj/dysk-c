#include "dbg.h"
#include "dysk_c.h"
#include "strutil.h"

#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

const string DEFAULT_FILE_PATH = "/proc/mounts";
const dspl_format DEFAULT_DISPLAY_FORMAT = DSC_DSPL_TABLE;

static bool validate_display_format(const string format, dspl_format *out) {
    const string allowed[] = {"json", "table"};

    for (size_t i = 0; i < sizeof(allowed) / sizeof(allowed[0]); i++) {
        if (strcmp(format, allowed[i]) == 0) {
            *out = (dspl_format)i;
            return true;
        }
    }

    return false;
}

int main(int argc, char *argv[]) {
    if (setlocale(LC_ALL, "") == NULL) log_warn("setlocale() failed");

    static const struct option options[] = {
        {"human-readable", no_argument, NULL, 'h'},
        {"display-format", required_argument, NULL, 'f'},
        {NULL, 0, NULL, 0},
    };

    prog_ops ops = {
        .path = DEFAULT_FILE_PATH,
        .display_format = DEFAULT_DISPLAY_FORMAT,
        .human_readable = false,
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "hf:", options, NULL)) != -1) {
        switch (opt) {

        case 'h':
            ops.human_readable = true;
            break;

        case 'f':
            if (!validate_display_format(optarg, &(ops.display_format))) {
                fprintf(stderr, "Invalid display format: '%s'\n", optarg);
                return EXIT_FAILURE;
            }
            break;

        default:
            fprintf(stderr, "Usage: dsc -[h] -[f FORMAT] [PATH]\n");
            break;
        }
    }

    if (optind < argc) ops.path = argv[optind];

    debug("display-format = %d", ops.display_format);
    debug("human-readable = %s", ops.human_readable ? "Yes" : "No");
    debug("path = %s", ops.path);

    int rc = run(ops);
    check_ret(fflush(stdout) == 0, EXIT_FAILURE, "fflush()");

    return rc;
}
