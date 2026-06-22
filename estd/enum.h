#ifndef __ESTD_ENUM_H__
#define __ESTD_ENUM_H__

#include "str.h"

#define ___ESTD_ENUM_NOVALUE_ELEMENT_APPLY(name) name
#define ___ESTD_ENUM_VALUE_ELEMENT_APPLY(name, value) name = value
#define ___ESTD_ENUM_ELEMENT_SEP ,

#define ___ESTD_ENUM_NOVALUE_TOSTRING_APPLY(name) \
    if (val == name) {                            \
        *result = ESTD_LITERAL(#name);            \
        return ESTD_SUCCESS;                      \
    }
#define ___ESTD_ENUM_VALUE_TOSTRING_APPLY(name, value) \
    if (val == name) {                                 \
        *result = ESTD_LITERAL(#name);                 \
        return ESTD_SUCCESS;                           \
    }
#define ___ESTD_ENUM_TOSTRING_SEP else

#define ___ESTD_ENUM_NOVALUE_FROMSTRING_APPLY(name)         \
    if (estdStringCompare(ESTD_LITERAL(#name), val) == 0) { \
        *result = name;                                     \
        return ESTD_SUCCESS;                                \
    }
#define ___ESTD_ENUM_VALUE_FROMSTRING_APPLY(name, value)    \
    if (estdStringCompare(ESTD_LITERAL(#name), val) == 0) { \
        *result = name;                                     \
        return ESTD_SUCCESS;                                \
    }
#define ___ESTD_ENUM_FROMSTRING_SEP else


#define ESTD_ENUM(typename, data)                                                                                      \
    typedef enum typename {                                                                                            \
        data(___ESTD_ENUM_NOVALUE_ELEMENT_APPLY, ___ESTD_ENUM_VALUE_ELEMENT_APPLY, ___ESTD_ENUM_ELEMENT_SEP)           \
    } typename;                                                                                                        \
    EstdResult estd##typename##ToString(EstdString* result, typename val) {                                            \
        data(___ESTD_ENUM_NOVALUE_TOSTRING_APPLY, ___ESTD_ENUM_VALUE_TOSTRING_APPLY, ___ESTD_ENUM_TOSTRING_SEP);       \
        ESTD_THROW(ESTD_INVALID_ENUM, "Invalid enum value %d", (int)val);                                              \
    }                                                                                                                  \
    EstdResult estd##typename##FromString(typename* result, EstdString val) {                                          \
        data(___ESTD_ENUM_NOVALUE_FROMSTRING_APPLY, ___ESTD_ENUM_VALUE_FROMSTRING_APPLY, ___ESTD_ENUM_FROMSTRING_SEP); \
        ESTD_THROW(ESTD_INVALID_ENUM, "Invalid enum string %" PRIestr, ESTD_STRING_ARG(val));                          \
    }

#endif
