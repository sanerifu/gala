#ifndef __GALA_COMMANDS_H__
#define __GALA_COMMANDS_H__

#include "common.h"
#include "engine.h"
#include "resource.h"

GALA_ENUM(GalaCommandType){
    GALA_COMMAND_TYPE_CREATE_PROGRAM,
    GALA_COMMAND_TYPE_CREATE_VERTEX_ARRAY,
    GALA_COMMAND_TYPE_BIND_PIPELINE,
    GALA_COMMAND_TYPE_BIND_VERTEX_ARRAY,
    GALA_COMMAND_TYPE_DRAW_ARRAYS
};

GALA_STRUCT(GalaCommandCreateProgram) {
    GalaProgram* program;
};

GALA_STRUCT(GalaCommandCreateVertexArray) {
    GalaVertexArray* vertex_array;
};

GALA_STRUCT(GalaCommandBindPipeline) {
    GalaPipeline* pipeline;
};

GALA_STRUCT(GalaCommandBindVertexArray) {
    GalaVertexArray* vertex_array;
};

GALA_STRUCT(GalaCommandDrawArrays) {
    int start;
    int count;
};

GALA_STRUCT(GalaCommand) {
    GalaCommandType type;
    char const* file;
    char const* func;
    int line;
    union {
        GalaCommandCreateProgram create_program;
        GalaCommandCreateVertexArray create_vertex_array;
        GalaCommandBindPipeline bind_pipeline;
        GalaCommandBindVertexArray bind_vertex_array;
        GalaCommandDrawArrays draw_arrays;
    };
};

GalaResult galaProcessCommand(GalaCommand* command);

#endif
