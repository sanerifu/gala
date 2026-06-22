#ifndef __ESTD_ARGS_H__
#define __ESTD_ARGS_H__

#include <iso646.h>
#include <stdbool.h>

#include "meta.h"
#include "str.h"

#define ___ESTD_ARGS_NAMED_HELP_GENERATOR(name, description, action) "\t--" #name "=\t" description "\n"
#define ___ESTD_ARGS_NAMED_HELP_SEPARATOR

#define ___ESTD_ARGS_POSITIONAL_HELP_GENERATOR(name, description, action) "\t" #name "\t" description "\n"
#define ___ESTD_ARGS_POSITIONAL_HELP_SEPARATOR

#define ___ESTD_ARGS_HELP_MESSAGE( \
    required_named_arguments,      \
    optional_named_arguments,      \
    required_positional_arguments, \
    optional_positional_arguments  \
)                                  \
    "Usage: %s [named arguments] (required positional arguments) [optional positional arguments]\n" \
    "Required Named Arguments:\n" \
    required_named_arguments(___ESTD_ARGS_NAMED_HELP_GENERATOR, ___ESTD_ARGS_NAMED_HELP_SEPARATOR) \
    "Required Positional Arguments:\n" \
    required_positional_arguments(___ESTD_ARGS_POSITIONAL_HELP_GENERATOR, ___ESTD_ARGS_POSITIONAL_HELP_SEPARATOR) \
    "Optional Named Arguments:\n" \
    optional_named_arguments(___ESTD_ARGS_NAMED_HELP_GENERATOR, ___ESTD_ARGS_NAMED_HELP_SEPARATOR) \
    "Optional Positional Arguments:\n" \
    optional_positional_arguments(___ESTD_ARGS_POSITIONAL_HELP_GENERATOR, ___ESTD_ARGS_POSITIONAL_HELP_SEPARATOR)

#define ___ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_GENERATOR(name, description, action) bool __estd_seen_##name = false;
#define ___ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_SEPARATOR

#define ___ESTD_ARGS_REQUIRED_VARIABLE_CHECK_GENERATOR(name, description, action) \
    if (not __estd_seen_##name) {                                                 \
        ESTD_THROW(ESTD_MISSING_ARGUMENT, "Missing argument %s", #name);          \
    }
#define ___ESTD_ARGS_REQUIRED_VARIABLE_CHECK_SEPARATOR

#define ___ESTD_ARGS_POSITIONAL_VARIABLE_GENERATOR(name, description, action) \
    int __estd_index_##name = __COUNTER__ - __estd_positional_counter_start;
#define ___ESTD_ARGS_POSITIONAL_VARIABLE_SEPARATOR

#define ___ESTD_ARGS_REQUIRED_NAMED_CHECKER_GENERATOR(name, description, action) \
    if (estdStringCompare(key, ESTD_LITERAL(#name)) == 0) {                      \
        __estd_seen_##name = true;                                               \
        ___ESTD_EVAL action;                                                     \
    }
#define ___ESTD_ARGS_REQUIRED_NAMED_CHECKER_SEPARATOR else

#define ___ESTD_ARGS_OPTIONAL_NAMED_CHECKER_GENERATOR(name, description, action) \
    if (estdStringCompare(key, ESTD_LITERAL(#name)) == 0) {                      \
        ___ESTD_EVAL action;                                                     \
    }
#define ___ESTD_ARGS_OPTIONAL_NAMED_CHECKER_SEPARATOR else

#define ___ESTD_ARGS_REQUIRED_POSITIONAL_CHECKER_GENERATOR(name, description, action)  \
    if (position - __estd_positional_start + 1 ==                                      \
        __estd_index_##name) { /* +1 is needed because __COUNTER__ hack starts at 0 */ \
        __estd_seen_##name = true;                                                     \
        ___ESTD_EVAL action;                                                           \
    }
#define ___ESTD_ARGS_REQUIRED_POSITIONAL_CHECKER_SEPARATOR else

#define ___ESTD_ARGS_OPTIONAL_POSITIONAL_CHECKER_GENERATOR(name, description, action) \
    if (position - __estd_positional_start + 1 == __estd_index_##name) {              \
        ___ESTD_EVAL action;                                                          \
    }
#define ___ESTD_ARGS_OPTIONAL_POSITIONAL_CHECKER_SEPARATOR else

/**
 * @brief Generator macro for argument parser for given argument handlers
 * @details This uses X-macro concept. Each argument declaration must be a macro taking two arguments: the per-argument
 * handler and separator The separator is just a macro, it is used as-is. It must be interleaved between argument
 * declarations The per-argument handler is another function taking 3 arguments.
 * 1. The identifier of the argument. Must be a valid C identifier. Can start with number. For named arguments, it is
 * checked as `--id` where `id` is the identifier. Positional arguments use the name only for information for help
 * message and errors. Still, they must obey same rules to be valid.
 * 2. The description of the argument. This must be a string literal It is used only for help message.
 * 3. The action for the argument. This must be enclosed in parantheses. It can be any statement. Named arguments have
 * `value` defined as the value of the argument. Positional arguments have `arg` defined as the argument itself. Both
 * are `EstdString`s. The resulting parser accepts first only named arguments, then only positional arguments. Named
 * arguments always start with `--`, followed by identifier, followed by `=` and the value. The `=` and value are
 * optional, if they don't exist the `value` variable will simply be empty. For example: `./program --flag1
 * --output=foo.txt --input=bar.txt compile` will apply actions of `flag1` (with empty value), `output` (with `foo.txt`
 * value), `input` (with `bar.txt` value) and the first positional argument (with `compile` arg).
 * @param name Name of the parser function to be generated
 * @param params Extra parameters passed to the parser function. Must have at least one parameter. Must be enclosed in
 * parantheses
 * @example
 * #define EXAMPLE_NAMED_ARGS(ARG, SEP) ARG(foo, "This is foo's description", (do_something_with_foo(value))) SEP
 * ARG(bar, "This is bar's description", ()) #define EXAMPLE_POS_ARGS(ARG, SEP) ARG(baz, "Baz, this is", (printf("Baz is
 * %" PRIestr "\n", ESTD_STRING_ARG(arg)))) SEP ARG(another, "", ()) ESTD_GENERATE_ARGUMENT_PARSER(my_parser_fun, (int),
 * EXAMPLE_NAMED_ARGS, ESTD_NONE, ESTD_NONE, EXAMPLE_POS_ARGS)
 * // which generates `my_parser_fun`, taking argc, argv and unnamed int. Can be used like:
 * ESTD_BUBBLE(my_parser_fun(argc, argv, 0), "Could not parse command line arguments");
 */
#define ESTD_GENERATE_ARGUMENT_PARSER(                                                                                                                          \
    name,                                                                                                                                                       \
    params,                                                                                                                                                     \
    required_named_arguments,                                                                                                                                   \
    optional_named_arguments,                                                                                                                                   \
    required_positional_arguments,                                                                                                                              \
    optional_positional_arguments                                                                                                                               \
)                                                                                                                                                               \
    EstdResult name(int argc, char* argv[argc], ___ESTD_EVAL params) {                                                                                          \
        if (argc == 1) {                                                                                                                                        \
            printf(                                                                                                                                             \
                ___ESTD_ARGS_HELP_MESSAGE(                                                                                                                      \
                    required_named_arguments,                                                                                                                   \
                    optional_named_arguments,                                                                                                                   \
                    required_positional_arguments,                                                                                                              \
                    optional_positional_arguments                                                                                                               \
                ),                                                                                                                                              \
                argv[0]                                                                                                                                         \
            );                                                                                                                                                  \
            return ESTD_SUCCESS;                                                                                                                                \
        }                                                                                                                                                       \
        required_named_arguments(                                                                                                                               \
            ___ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_GENERATOR,                                                                                               \
            ___ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_SEPARATOR                                                                                                \
        );                                                                                                                                                      \
        required_positional_arguments(                                                                                                                          \
            ___ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_GENERATOR,                                                                                               \
            ___ESTD_ARGS_REQUIRED_VARIABLE_DECLARATION_SEPARATOR                                                                                                \
        );                                                                                                                                                      \
        int position;                                                                                                                                           \
        int __estd_positional_start = 0;                                                                                                                        \
        for (position = 1; position < argc; position++) {                                                                                                       \
            EstdString arg = ESTD_CTRING(argv[position]);                                                                                                       \
            if (not estdStringHasPrefix(arg, ESTD_LITERAL("--"))) {                                                                                             \
                __estd_positional_start = position;                                                                                                             \
                break;                                                                                                                                          \
            }                                                                                                                                                   \
            EstdString __estd_shifted = ESTD_SLICE(arg, 2, arg.length);                                                                                         \
            EstdString value = __estd_shifted;                                                                                                                  \
            EstdString key = estdStringSplit(&value, ESTD_LITERAL("="));                                                                                        \
            required_named_arguments(                                                                                                                           \
                ___ESTD_ARGS_REQUIRED_NAMED_CHECKER_GENERATOR,                                                                                                  \
                ___ESTD_ARGS_REQUIRED_NAMED_CHECKER_SEPARATOR                                                                                                   \
            ) else optional_named_arguments(___ESTD_ARGS_OPTIONAL_NAMED_CHECKER_GENERATOR, ___ESTD_ARGS_OPTIONAL_NAMED_CHECKER_SEPARATOR) else {                \
                ESTD_THROW(ESTD_UNKNOWN_ARGUMENT, "Unknown argument: %s\n", argv[position]);                                                                    \
            }                                                                                                                                                   \
        }                                                                                                                                                       \
        int __estd_positional_counter_start = __COUNTER__;                                                                                                      \
        required_positional_arguments(                                                                                                                          \
            ___ESTD_ARGS_POSITIONAL_VARIABLE_GENERATOR,                                                                                                         \
            ___ESTD_ARGS_POSITIONAL_VARIABLE_SEPARATOR                                                                                                          \
        );                                                                                                                                                      \
        optional_positional_arguments(                                                                                                                          \
            ___ESTD_ARGS_POSITIONAL_VARIABLE_GENERATOR,                                                                                                         \
            ___ESTD_ARGS_POSITIONAL_VARIABLE_SEPARATOR                                                                                                          \
        );                                                                                                                                                      \
        for (; position < argc; position++) {                                                                                                                   \
            EstdString arg = ESTD_CTRING(argv[position]);                                                                                                       \
            required_positional_arguments(                                                                                                                      \
                ___ESTD_ARGS_REQUIRED_POSITIONAL_CHECKER_GENERATOR,                                                                                             \
                ___ESTD_ARGS_REQUIRED_POSITIONAL_CHECKER_SEPARATOR                                                                                              \
            ) else optional_positional_arguments(___ESTD_ARGS_OPTIONAL_POSITIONAL_CHECKER_GENERATOR, ___ESTD_ARGS_OPTIONAL_POSITIONAL_CHECKER_SEPARATOR) else { \
                ESTD_THROW(ESTD_UNKNOWN_ARGUMENT, "Unknown extra argument: %s\n", argv[position]);                                                              \
            }                                                                                                                                                   \
        }                                                                                                                                                       \
        required_named_arguments(                                                                                                                               \
            ___ESTD_ARGS_REQUIRED_VARIABLE_CHECK_GENERATOR,                                                                                                     \
            ___ESTD_ARGS_REQUIRED_VARIABLE_CHECK_SEPARATOR                                                                                                      \
        );                                                                                                                                                      \
        required_positional_arguments(                                                                                                                          \
            ___ESTD_ARGS_REQUIRED_VARIABLE_CHECK_GENERATOR,                                                                                                     \
            ___ESTD_ARGS_REQUIRED_VARIABLE_CHECK_SEPARATOR                                                                                                      \
        );                                                                                                                                                      \
        return ESTD_SUCCESS;                                                                                                                                    \
    }

#endif