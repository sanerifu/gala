#ifndef __GALA_ENGINE_H__
#define __GALA_ENGINE_H__

#include <estd/arena.h>
#include <stddef.h>
#include <stdint.h>

typedef enum GalaResult {
    GALA_SUCCESS,
    GALA_RESULT_OUT_OF_MEMORY = ESTD_OUT_OF_MEMORY,
    GALA_RESULT_SHADER_NOT_FOUND,
    GALA_RESULT_INVALID_ATTRIBUTE,
    GALA_RESULT_OPENGL_ERROR,
    GALA_RESULT_SHADER_COMPILATION_ERROR,
    GALA_RESULT_PROGRAM_LINKAGE_ERROR
} GalaResult;

typedef struct GalaEngineConfig {
    size_t model_count;
} GalaEngineConfig;

typedef struct GalaEngine GalaEngine;

GalaResult galaCreateEngine(GalaEngine** o_self, GalaEngineConfig config, EstdArena** allocator);

#endif
