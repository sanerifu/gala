#ifndef __GALA_COMMON_H__
#define __GALA_COMMON_H__

#define ___GALA_CONCAT2(a, b) a##b
#define ___GALA_CONCAT(a, b) ___GALA_CONCAT2(a, b)
#define ___GALA_BLANK
#define ___GALA_COMMA ,

#define ___GALA_STRUCT_FIELD(T, name) T name;
#define GALA_REFLECTED_STRUCT(name, fields)               \
    typedef struct name {                                 \
        fields(name, ___GALA_STRUCT_FIELD, ___GALA_BLANK) \
    } name;

#define ___GALA_ENUM_VALUE(name, value) name = value
#define ___GALA_ENUM_ENUM(name) name
#define GALA_REFLECTED_ENUM(values)                                          \
    typedef enum ___GALA_CONCAT(values, _typename) {                         \
        values(values, ___GALA_ENUM_ENUM, ___GALA_ENUM_VALUE, ___GALA_COMMA) \
    } ___GALA_CONCAT(values, _typename);

#define GALA_STRUCT(name)     \
    typedef struct name name; \
    struct name
#define GALA_ENUM(name)     \
    typedef enum name name; \
    enum name

#endif
