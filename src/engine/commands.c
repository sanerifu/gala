#include "commands.h"

#include <estd/result.h>
#include <estd/string_builder.h>
#include <glad/glad.h>

#include "glresult.h"

static void fcloseWrapper(void* f) {
    FILE* fp = *(FILE**)f;
    fclose(fp);
}

static GalaResult galaGetAttributeType(EstdString* o_ret, GalaAttribute const* attr) {
    EstdString type_specifier;
    switch (attr->type) {
        case GALA_ATTRIBUTE_TYPE_INTEGER:
            if (attr->element_type == GALA_TYPE_S8 || attr->element_type == GALA_TYPE_S16 ||
                attr->element_type == GALA_TYPE_S32) {
                if (attr->count == 1) {
                    type_specifier = ESTD_LITERAL("int");
                } else if (attr->count == 2) {
                    type_specifier = ESTD_LITERAL("ivec2");
                } else if (attr->count == 3) {
                    type_specifier = ESTD_LITERAL("ivec3");
                } else if (attr->count == 4) {
                    type_specifier = ESTD_LITERAL("ivec4");
                } else {
                    ESTD_THROW(
                        GALA_RESULT_INVALID_ATTRIBUTE,
                        "Invalid number of elements for signed integer attribute: %d",
                        attr->count
                    );
                }
            } else if (
                attr->element_type == GALA_TYPE_U8 || attr->element_type == GALA_TYPE_U16 ||
                attr->element_type == GALA_TYPE_U32
            ) {
                if (attr->count == 1) {
                    type_specifier = ESTD_LITERAL("uint");
                } else if (attr->count == 2) {
                    type_specifier = ESTD_LITERAL("uvec2");
                } else if (attr->count == 3) {
                    type_specifier = ESTD_LITERAL("uvec3");
                } else if (attr->count == 4) {
                    type_specifier = ESTD_LITERAL("uvec4");
                } else {
                    ESTD_THROW(GALA_RESULT_INVALID_ATTRIBUTE, "Unsigned integer attribute with count %d", attr->count);
                }
            } else {
                ESTD_THROW(GALA_RESULT_INVALID_ATTRIBUTE, "floating-point element type for integer attribute");
            }
            break;
        case GALA_ATTRIBUTE_TYPE_FLOAT:
        case GALA_ATTRIBUTE_TYPE_NORMALIZED:
            if (attr->count == 1) {
                type_specifier = ESTD_LITERAL("float");
            } else if (attr->count == 2) {
                type_specifier = ESTD_LITERAL("vec2");
            } else if (attr->count == 3) {
                type_specifier = ESTD_LITERAL("vec3");
            } else if (attr->count == 4) {
                type_specifier = ESTD_LITERAL("vec4");
            } else {
                ESTD_THROW(GALA_RESULT_INVALID_ATTRIBUTE, "floating point attribute with count %d", attr->count);
            }
            break;
    }
    *o_ret = type_specifier;
    return GALA_SUCCESS;
}

static GalaResult galaMakeShader(GLuint* o_shader, EstdString source, GLenum type) {
    GLint success;
    char info_log[512];

    GLuint shader;
    GALA_GLOP(shader = glCreateShader(type), "shader creation");
    GALA_GLOP(glShaderSource(shader, 1, (char const* const*)&source.data, NULL), "shader source upload");
    GALA_GLOP(glCompileShader(shader), "shader compilation");

    GALA_GLOP(glGetShaderiv(shader, GL_COMPILE_STATUS, &success), "querying shader compilation status");
    if (!success) {
        GALA_GLOP(glGetShaderInfoLog(shader, 512, NULL, info_log), "retrieving shader info log");
        ESTD_THROW(GALA_RESULT_SHADER_COMPILATION_ERROR, "shader compilation: %s", info_log);
    }
    *o_shader = shader;
    return GALA_SUCCESS;
}

static GalaResult galaMakeProgram(GLuint* o_program, GLuint vertex_shader, GLuint fragment_shader) {
    GLint success;
    char info_log[512];

    GLuint program;
    GALA_GLOP(program = glCreateProgram(), "program creation");
    GALA_GLOP(glAttachShader(program, vertex_shader), "attaching vertex shader to program");
    GALA_GLOP(glAttachShader(program, fragment_shader), "attaching fragment shader to program");
    GALA_GLOP(glLinkProgram(program), "linking program");

    GALA_GLOP(glGetProgramiv(program, GL_LINK_STATUS, &success), "querying program success");
    if (!success) {
        GALA_GLOP(glGetProgramInfoLog(program, 512, NULL, info_log), "retrieving program info log");
        ESTD_THROW(GALA_RESULT_PROGRAM_LINKAGE_ERROR, "linking program: %s", info_log);
    }

    GALA_GLOP(glDeleteShader(vertex_shader), "deleting vertex shader");
    GALA_GLOP(glDeleteShader(fragment_shader), "deleting fragment shader");

    *o_program = program;
    return GALA_SUCCESS;
}

static GalaResult galaCreateProgram(GalaProgram* io_program, EstdArena** allocator) {
    GalaProgram program = *io_program;
    ESTD_CLEAN(estdArenaDestroy) EstdArena* arena = NULL;
    EstdString filename;
    ESTD_OP(
        estdStringFormat(&filename, &arena, "shaders/%" PRIestr ".glsl", ESTD_STRING_ARG(program.name)),
        "creating filename for shader %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );
    ESTD_CLEAN(fclose) FILE* file = fopen(filename.data, "rb");
    ESTD_OP(
        estdReadFile(&program.source, allocator, file),
        "reading program %" PRIestr " file %" PRIestr,
        ESTD_STRING_ARG(program.name),
        ESTD_STRING_ARG(filename)
    );

    EstdStringBuilder* vertex_source_builder = NULL;
    ESTD_OP(
        estdStringBuilderAppend(
            &vertex_source_builder,
            ESTD_LITERAL(
                "#version 410 core\n"
                "#define VERTEX 1\n"
                "#define VARYING out\n"
            ),
            &arena
        ),
        "prepending prelude to vertex shader %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    for (size_t a = 0; a < program.attribute_count; a++) {
        GalaAttribute const* attr = &program.attributes[a];
        EstdString type_specifier;
        ESTD_OP(
            galaGetAttributeType(&type_specifier, attr),
            "getting attribute type of attribute %" PRIestr " in program %" PRIestr,
            ESTD_STRING_ARG(attr->name),
            ESTD_STRING_ARG(program.name)
        );

        ESTD_OP(
            estdStringBuilderAppendf(
                &vertex_source_builder,
                &arena,
                "layout(location = %zu) in %" PRIestr " %" PRIestr ";\n",
                a,
                ESTD_STRING_ARG(type_specifier),
                ESTD_STRING_ARG(attr->name)
            ),
            "prepending attribute %" PRIestr " to source string for shader %" PRIestr,
            ESTD_STRING_ARG(attr->name),
            ESTD_STRING_ARG(program.name)
        );
    }

    ESTD_OP(
        estdStringBuilderAppendf(
            &vertex_source_builder,
            &arena,
            "#line 10001\n"
            "%" PRIestr,
            ESTD_STRING_ARG(program.source)
        ),
        "appending source %" PRIestr " to vertex shader source",
        ESTD_STRING_ARG(program.name)
    );

    EstdString vertex_source;
    ESTD_OP(
        estdStringBuilderBuild(&vertex_source, &vertex_source_builder, &arena),
        "building vertex shader source for %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    EstdStringBuilder* fragment_source_builder = NULL;
    ESTD_OP(
        estdStringBuilderAppendf(
            &fragment_source_builder,
            &arena,
            "#version 410 core\n"
            "#define FRAGMENT 1\n"
            "#define VARYING in\n"
            "#line 20001\n"
            "%" PRIestr,
            ESTD_STRING_ARG(program.source)
        ),
        "prepending prelude to fragment shader %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    EstdString fragment_source;
    ESTD_OP(
        estdStringBuilderBuild(&fragment_source, &fragment_source_builder, &arena),
        "building fragment shader source for %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    GLuint vertex_shader, fragment_shader;
    ESTD_OP(
        galaMakeShader(&vertex_shader, vertex_source, GL_VERTEX_SHADER),
        "creating vertex shader for program %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    ESTD_OP(
        galaMakeShader(&fragment_shader, fragment_source, GL_FRAGMENT_SHADER),
        "creating fragment shader for program %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    ESTD_OP(
        galaMakeProgram(&program.gl, vertex_shader, fragment_shader),
        "creating program %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    *io_program = program;
    return GALA_SUCCESS;
}

// TODO: Add buffers
static GalaResult galaCreateVertexArray(GalaVertexArray* vertex_array) {
    GALA_GLOP(glGenVertexArrays(1, &vertex_array->gl), "generating vertex array");
    GALA_GLOP(glBindVertexArray(vertex_array->gl), "binding vertex array");
    for (size_t a = 0; a < vertex_array->attribute_count; a++) {
        GalaAttribute const* attr = &vertex_array->attributes[a];
        GALA_GLOP(glEnableVertexAttribArray(a), "enabling attribute %zu", a);
        GLboolean normalized = GL_FALSE;
        // TODO: fix strides by summing
        switch (attr->type) {
            case GALA_ATTRIBUTE_TYPE_INTEGER:
                glVertexAttribIPointer(a, attr->count, attr->element_type, 0, NULL);
                break;
            case GALA_ATTRIBUTE_TYPE_NORMALIZED:
                normalized = GL_TRUE;
            case GALA_ATTRIBUTE_TYPE_FLOAT:
                glVertexAttribPointer(a, attr->count, attr->element_type, normalized, 0, NULL);
                break;
        }
    }
    GALA_GLOP(glBindVertexArray(0), "unbinding vertex array");
    return GALA_SUCCESS;
}

static GalaResult galaBindPipeline(GalaPipeline* pipeline) {
    GALA_GL(glUseProgram(pipeline->program->gl), "using program");
    return GALA_SUCCESS;
}

static GalaResult galaBindVertexArray(GalaVertexArray* vertex_array) {
    GALA_GL(glBindVertexArray(vertex_array->gl), "binding vertex array");
    return GALA_SUCCESS;
}

static GalaResult galaDrawArrays(int start, int count) {
    GALA_GL(glDrawArrays(GL_TRIANGLES, start, count), "drawing arrays");
    return GALA_SUCCESS;
}

GalaResult galaProcessCommand(GalaCommand* command) {
    EstdArena* arena;
    switch (command->type) {
        case GALA_COMMAND_TYPE_CREATE_PROGRAM:
            ESTD_BUBBLE(galaCreateProgram(command->create_program.program, &arena), "creating program");
            break;
        case GALA_COMMAND_TYPE_CREATE_VERTEX_ARRAY:
            ESTD_BUBBLE(galaCreateVertexArray(command->create_vertex_array.vertex_array), "creating vertex array");
            break;
        case GALA_COMMAND_TYPE_BIND_PIPELINE:
            ESTD_BUBBLE(galaBindPipeline(command->bind_pipeline.pipeline), "binding pipeline");
            break;
        case GALA_COMMAND_TYPE_BIND_VERTEX_ARRAY:
            ESTD_BUBBLE(galaBindVertexArray(command->bind_vertex_array.vertex_array), "binding vertex array");
            break;
        case GALA_COMMAND_TYPE_DRAW_ARRAYS:
            ESTD_BUBBLE(galaDrawArrays(command->draw_arrays.start, command->draw_arrays.count), "drawing arrays");
            break;
    }
    return GALA_SUCCESS;
}
