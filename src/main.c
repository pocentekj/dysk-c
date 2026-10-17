#include "dbg.h"
#include "deferred.h"
#include <stdlib.h>

int main(void) {
    deferred(close_fp) FILE *fp = fopen("README.md", "r");
    check_ret(fp, EXIT_FAILURE, "Failed to open README.md");

    deferred(free_str) char *line;
    size_t maxlen = 0;
    ssize_t nread = 0;

    while ((nread = getline(&line, &maxlen, fp)) != -1) {
        printf("%s", line);
    }

    return EXIT_SUCCESS;
}
