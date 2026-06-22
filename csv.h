#ifndef __ESTD_CSV_H__
#define __ESTD_CSV_H__

#include <stdbool.h>

#include "meta.h"
#include "str.h"

#define ESTD_GENERATE_CSV_PARSER(name, params, action, column_separator, row_separator, escape) \
    EstdResult name(EstdString data, ___ESTD_EVAL params) {                                     \
        if (data.length == 0) {                                                                 \
            return ESTD_SUCCESS;                                                                \
        }                                                                                       \
        int row = 0;                                                                            \
        int column = 0;                                                                         \
        size_t cell_start = 0;                                                                  \
        for (size_t i = 0; i < data.length; i++) {                                              \
            if (data.data[i] == column_separator) {                                             \
                EstdString cell = ESTD_SLICE(data, cell_start, i);                              \
                ___ESTD_EVAL action;                                                            \
                column += 1;                                                                    \
                cell_start = i + 1;                                                             \
            } else if (data.data[i] == row_separator) {                                         \
                EstdString cell = ESTD_SLICE(data, cell_start, i);                              \
                ___ESTD_EVAL action;                                                            \
                cell_start = i + 1;                                                             \
                column = 0;                                                                     \
                row += 1;                                                                       \
            } else if (data.data[i] == escape) {                                                \
                for (i += 1; i < data.length && data.data[i] != escape; i++) {                  \
                }                                                                               \
            }                                                                                   \
        }                                                                                       \
        if (data.data[data.length - 1] != row_separator) {                                      \
            EstdString cell = ESTD_SLICE(data, cell_start, data.length);                        \
            ___ESTD_EVAL action;                                                                \
        }                                                                                       \
        return ESTD_SUCCESS;                                                                    \
    }

#endif
