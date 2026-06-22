#ifndef __ESTD_RESULT_H__
#define __ESTD_RESULT_H__

#include <stdlib.h>

#include "log.h"

typedef enum {
    ESTD_SUCCESS,
    ESTD_OUT_OF_MEMORY,
    ESTD_INVALID_PERCENT_ENCODING,
    ESTD_ILLEGAL_NUMBER,
    ESTD_OVERFLOW,
    ESTD_IO_ERROR,
    ESTD_MISSING_ARGUMENT,
    ESTD_UNKNOWN_ARGUMENT,
    ESTD_INVALID_ENUM
} EstdResult;

#define ESTD_THROW(result, fmt, ...)                 \
    do {                                             \
        ESTD_ERROR(#result ": " fmt, ##__VA_ARGS__); \
        return result;                               \
    } while (0)

#define ESTD_BUBBLE(expr, fmt, ...)                                       \
    do {                                                                  \
        EstdResult ___estdmacro_result;                                   \
        if ((___estdmacro_result = (EstdResult)(expr)) != ESTD_SUCCESS) { \
            ESTD_TRACE(fmt, ##__VA_ARGS__);                               \
            return ___estdmacro_result;                                   \
        }                                                                 \
    } while (0)

#define ESTD_ASSERT(result, expr, fmt, ...)                                           \
    do {                                                                              \
        if (!(expr)) {                                                                \
            ESTD_ASSERTION("Assertion (" #expr ") " #result ": " fmt, ##__VA_ARGS__); \
            return result;                                                            \
        }                                                                             \
    } while (0)

#define ESTD_ASSERT_PANIC(expr, fmt, ...)                                       \
    do {                                                                        \
        if (!(expr)) {                                                          \
            ESTD_ASSERTION("Aborting due to (" #expr "): " fmt, ##__VA_ARGS__); \
            abort();                                                            \
        }                                                                       \
    } while (0)

#endif
