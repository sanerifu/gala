#ifndef __ESTD_LOG_H__
#define __ESTD_LOG_H__

#include <stddef.h>
#include <stdio.h>
#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>

// #if defined(_MSC_VER)
// #define GETTIME(to, fmt)                               \
//     do {                                               \
//         time_t ___now = time(NULL);                    \
//         struct tm ___tminfo;                           \
//         if (gmtime_s(&___tminfo, &___now) == 0) {      \
//             strftime(to, sizeof(to), fmt, &___tminfo); \
//         }                                              \
//     } while (0)
// #elif defined(__STDC_LIB_EXT1__) || (__STDC_VERSION__ >= 201112L)
// #define GETTIME(to, fmt)                               \
//     do {                                               \
//         time_t ___now = time(NULL);                    \
//         struct tm ___tminfo;                           \
//         if (gmtime_s(&___now, &___tminfo) != NULL) {   \
//             strftime(to, sizeof(to), fmt, &___tminfo); \
//         }                                              \
//     } while (0)
// #else
#define GETTIME(to, fmt)                              \
    do {                                              \
        time_t ___now = time(NULL);                   \
        struct tm* ___tminfo = gmtime(&___now);       \
        if (___tminfo) {                              \
            strftime(to, sizeof(to), fmt, ___tminfo); \
        }                                             \
    } while (0)
// #endif

#define ESTD_LOG(file, color, reset, tag, fmt, ...)            \
    do {                                                       \
        char ___estdmacro_time[32] = "0000-00-00 00:00:00";    \
        GETTIME(___estdmacro_time, "%Y-%m-%d %H:%M:%S");       \
        fprintf(                                               \
            file,                                              \
            color "[%s]" tag " [%s @ %s:%d]: " fmt reset "\n", \
            ___estdmacro_time,                                 \
            __func__,                                          \
            __FILE__,                                          \
            __LINE__,                                          \
            ##__VA_ARGS__                                      \
        );                                                     \
    } while (0)

#define ESTD_DEBUG(fmt, ...) ESTD_LOG(stderr, "\x1b[2m", "\x1b[0m", " (DBG)", fmt, ##__VA_ARGS__)
#define ESTD_TRACE(fmt, ...) ESTD_LOG(stderr, "\x1b[95m", "\x1b[0m", " (TRC)", fmt, ##__VA_ARGS__)
#define ESTD_INFO(fmt, ...) ESTD_LOG(stderr, "\x1b[96m", "\x1b[0m", " (INF)", fmt, ##__VA_ARGS__)
#define ESTD_WARNING(fmt, ...) ESTD_LOG(stderr, "\x1b[93m", "\x1b[0m", " (WRN)", fmt, ##__VA_ARGS__)
#define ESTD_ERROR(fmt, ...) ESTD_LOG(stderr, "\x1b[91m", "\x1b[0m", " (ERR)", fmt, ##__VA_ARGS__)
#define ESTD_FATAL(fmt, ...) ESTD_LOG(stderr, "\x1b[101m", "\x1b[0m", " (FTL)", fmt, ##__VA_ARGS__)
#define ESTD_ASSERTION(fmt, ...) ESTD_LOG(stderr, "\x1b[105m", "\x1b[0m", " (AST)", fmt, ##__VA_ARGS__)

#endif
