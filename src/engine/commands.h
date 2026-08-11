#ifndef __GALA_COMMANDS_H__
#define __GALA_COMMANDS_H__

#include "common.h"
#include "engine.h"
#include "resource.h"

GALA_STRUCT(GalaCommand) {
    char const* file;
    char const* func;
    int line;
    enum {
        GALA_COMMAND_TYPE_CREATE_PROGRAM,
        GALA_COMMAND_TYPE_CREATE_VERTEX_ARRAY,
        GALA_COMMAND_TYPE_BIND_PIPELINE,
        GALA_COMMAND_TYPE_BIND_VERTEX_ARRAY,
        GALA_COMMAND_TYPE_DRAW_ARRAYS
    } type;
    union {
        struct {
            GalaProgram* program;
        } create_program;
        struct {
            GalaVertexArray* vertex_array;
        } create_vertex_array;
        struct {
            GalaPipeline* pipeline;
        } bind_pipeline;
        struct {
            GalaVertexArray* vertex_array;
        } bind_vertex_array;
        struct {
            int start;
            int count;
        } draw_arrays;
    };
};

GalaResult galaProcessCommand(GalaCommand* command);

#endif
