#ifndef __GALA_ENGINE_H__
#define __GALA_ENGINE_H__

#include <estd/arena.h>
#include <stddef.h>
#include <stdint.h>

#include "common.h"

#define GALA_SUCCESS NULL
#define ___GALA_RESULTS(RESULT, SEP)                                           \
    RESULT(GALA_RESULT_OUT_OF_MEMORY) SEP RESULT(GALA_RESULT_SHADER_NOT_FOUND) \
    SEP RESULT(GALA_RESULT_INVALID_ATTRIBUTE)                                  \
    SEP RESULT(GALA_RESULT_OPENGL_ERROR)                                       \
    SEP RESULT(GALA_RESULT_SHADER_COMPILATION_ERROR)                           \
    SEP RESULT(GALA_RESULT_PROGRAM_LINKAGE_ERROR)
ESTD_RESULT(GalaResult, ___GALA_RESULTS);

GALA_STRUCT(GalaEngineConfig) {
    size_t model_count;
};

typedef struct GalaEngine GalaEngine;

GalaResult galaCreateEngine(GalaEngine** o_self, GalaEngineConfig config, EstdArena** allocator);
GalaResult galaUpdateEngine(GalaEngine* self, EstdArena** allocator);

#endif
