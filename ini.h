#ifndef __ESTD_INI_H__
#define __ESTD_INI_H__

#include "meta.h"
#include "result.h"
#include "str.h"

#define ESTD_GENERATE_INI_PARSER(                                                                 \
    name,                                                                                         \
    params,                                                                                       \
    section_action,                                                                               \
    entry_action,                                                                                 \
    key_value_separator,                                                                          \
    line_separator,                                                                               \
    comment_start,                                                                                \
    starting_section                                                                              \
)                                                                                                 \
    EstdResult name(EstdString data, ___ESTD_EVAL params) {                                       \
        EstdString raw_line;                                                                      \
        EstdString section = ESTD_LITERAL(starting_section);                                      \
        while ((raw_line = estdStringSplit(&data, ESTD_LITERAL(line_separator))).data) {          \
            raw_line = estdStringSplit(&raw_line, ESTD_CHAR(comment_start));                      \
            EstdString line = estdStringTrim(raw_line);                                           \
            if (line.length == 0) {                                                               \
                continue;                                                                         \
            }                                                                                     \
            if (line.data[0] == '[' && line.data[line.length - 1] == ']') {                       \
                section = ESTD_SLICE(line, 1, line.length - 1);                                   \
                ___ESTD_EVAL section_action;                                                      \
            } else {                                                                              \
                EstdString raw_value = raw_line;                                                  \
                EstdString raw_key = estdStringSplit(&raw_value, ESTD_CHAR(key_value_separator)); \
                EstdString key = estdStringTrim(raw_key);                                         \
                EstdString value = estdStringTrim(raw_value);                                     \
                ___ESTD_EVAL entry_action;                                                        \
            }                                                                                     \
        }                                                                                         \
        return ESTD_SUCCESS;                                                                      \
    }

#endif
