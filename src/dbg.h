#pragma once

/**
 * Inspired by macros from the book "Learn C The Hard Way" by Zed A. Shaw
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
    #define debug(M, ...)
#else
    #define debug(M, ...)                                                                          \
        fprintf(stderr, "[DEBUG] (%s:%d): " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define log_err(M, ...)                                                                            \
    fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__,                      \
            clean_errno(), ##__VA_ARGS__)
#define log_warn(M, ...)                                                                           \
    fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__,                       \
            clean_errno(), ##__VA_ARGS__)
#define log_info(M, ...)                                                                           \
    fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define check(A, M, ...)                                                                           \
    do {                                                                                           \
        if (!(A)) {                                                                                \
            log_err(M, ##__VA_ARGS__);                                                             \
            errno = 0;                                                                             \
            goto error;                                                                            \
        }                                                                                          \
    } while (0)

#define check_mem(A) check((A), "Out of memory.")

#define check_ret(A, R, M, ...)                                                                    \
    do {                                                                                           \
        if (!(A)) {                                                                                \
            log_err(M, ##__VA_ARGS__);                                                             \
            return R;                                                                              \
        }                                                                                          \
    } while (0)

#define check_nret(A, M, ...)                                                                      \
    do {                                                                                           \
        if (!(A)) {                                                                                \
            log_err(M, ##__VA_ARGS__);                                                             \
            return;                                                                                \
        }                                                                                          \
    } while (0)
