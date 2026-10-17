#pragma once

#include <stdio.h>
#include <stdlib.h>

#if defined(__GNUC__) || defined(__clang__)
    #define deferred(fn) __attribute__((cleanup(fn)))
#else
    #error "deferred() requires GCC or Clang"
#endif

static inline void close_fp(FILE **fp) {
    if (!fp || !*fp || *fp == stdin) return;
    fclose(*fp);
    *fp = NULL;
}

static inline void free_str(char **str) {
    if (!str || !*str) return;
    free(*str);
    *str = NULL;
}
