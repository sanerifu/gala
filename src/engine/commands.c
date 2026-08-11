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
                    ESTD_THROW(
                        GALA_RESULT_INVALID_ATTRIBUTE,
                        "Invalid number of elements for unsigned integer attribute: %d",
                        attr->count
                    );
                }
            } else {
                ESTD_THROW(GALA_RESULT_INVALID_ATTRIBUTE, "Cannot use floating-point numbers for integer attribute");
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
                ESTD_THROW(
                    GALA_RESULT_INVALID_ATTRIBUTE,
                    "Invalid number of elements for floating point attribute: %d",
                    attr->count
                );
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
    GALA_GL(shader = glCreateShader(type), "Could not create shader");
    GALA_GL(glShaderSource(shader, 1, (char const* const*)&source.data, NULL), "Could not give source to shader");
    GALA_GL(glCompileShader(shader), "Could not compile shader");

    GALA_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success), "Could not get shader success");
    if (!success) {
        GALA_GL(glGetShaderInfoLog(shader, 512, NULL, info_log), "Could not get shader info log");
        ESTD_THROW(GALA_RESULT_SHADER_COMPILATION_ERROR, "Could not compile shader: %s", info_log);
    }
    *o_shader = shader;
    return GALA_SUCCESS;
}

static GalaResult galaMakeProgram(GLuint* o_program, GLuint vertex_shader, GLuint fragment_shader) {
    GLint success;
    char info_log[512];

    GLuint program;
    GALA_GL(program = glCreateProgram(), "Could not create program");
    GALA_GL(glAttachShader(program, vertex_shader), "Could not attach vertex shader to program");
    GALA_GL(glAttachShader(program, fragment_shader), "Could not attach fragment shader to program");
    GALA_GL(glLinkProgram(program), "Could not link program");

    GALA_GL(glGetProgramiv(program, GL_LINK_STATUS, &success), "Could not get program success");
    if (!success) {
        GALA_GL(glGetProgramInfoLog(program, 512, NULL, info_log), "Could not get program info log");
        ESTD_THROW(GALA_RESULT_PROGRAM_LINKAGE_ERROR, "Could not link program: %s", info_log);
    }

    GALA_GL(glDeleteShader(vertex_shader), "Could not delete vertex shader");
    GALA_GL(glDeleteShader(fragment_shader), "Could not delete fragment shader");

    *o_program = program;
    return GALA_SUCCESS;
}

static GalaResult galaCreateProgram(GalaProgram* io_program, EstdArena** allocator) {
    GalaProgram program = *io_program;
    ESTD_CLEAN(estdArenaDestroy) EstdArena* arena = NULL;
    EstdString filename;
    ESTD_BUBBLE_T(
        GalaResult,
        estdStringFormat(&filename, &arena, "shaders/%" PRIestr ".glsl", ESTD_STRING_ARG(program.name)),
        "Could not create filename for shader %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );
    ESTD_CLEAN(fclose) FILE* file = fopen(filename.data, "rb");
    ESTD_BUBBLE_T(
        GalaResult,
        estdReadFile(&program.source, allocator, file),
        "Could not read program %" PRIestr " file %" PRIestr,
        ESTD_STRING_ARG(program.name),
        ESTD_STRING_ARG(filename)
    );

    EstdStringBuilder* vertex_source_builder = NULL;
    ESTD_BUBBLE_T(
        GalaResult,
        estdStringBuilderAppend(
            &vertex_source_builder,
            ESTD_LITERAL(
                "#version 410 core\n"
                "#define VERTEX 1\n"
                "#define VARYING out\n"
            ),
            &arena
        ),
        "Could not prepend prelude to vertex shader %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    for (size_t a = 0; a < program.attribute_count; a++) {
        GalaAttribute const* attr = &program.attributes[a];
        EstdString type_specifier;
        ESTD_BUBBLE_T(
            GalaResult,
            galaGetAttributeType(&type_specifier, attr),
            "Could not get attribute type of attribute %" PRIestr " in program %" PRIestr,
            ESTD_STRING_ARG(attr->name),
            ESTD_STRING_ARG(program.name)
        );

        ESTD_BUBBLE_T(
            GalaResult,
            estdStringBuilderAppendf(
                &vertex_source_builder,
                &arena,
                "layout(location = %zu) in %" PRIestr " %" PRIestr ";\n",
                a,
                ESTD_STRING_ARG(type_specifier),
                ESTD_STRING_ARG(attr->name)
            ),
            "Could not prepend attribute %" PRIestr " to source string for shader %" PRIestr,
            ESTD_STRING_ARG(attr->name),
            ESTD_STRING_ARG(program.name)
        );
    }

    ESTD_BUBBLE_T(
        GalaResult,
        estdStringBuilderAppendf(
            &vertex_source_builder,
            &arena,
            "#line 10001\n"
            "%" PRIestr,
            ESTD_STRING_ARG(program.source)
        ),
        "Could not append source %" PRIestr " to vertex shader source",
        ESTD_STRING_ARG(program.name)
    );

    EstdString vertex_source;
    ESTD_BUBBLE_T(
        GalaResult,
        estdStringBuilderBuild(&vertex_source, &vertex_source_builder, &arena),
        "Could not build vertex shader source for %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );
    ESTD_DEBUG("Vertex: %" PRIestr, ESTD_STRING_ARG(vertex_source));

    EstdStringBuilder* fragment_source_builder = NULL;
    ESTD_BUBBLE_T(
        GalaResult,
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
        "Could not prepend prelude to fragment shader %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    EstdString fragment_source;
    ESTD_BUBBLE_T(
        GalaResult,
        estdStringBuilderBuild(&fragment_source, &fragment_source_builder, &arena),
        "Could not build fragment shader source for %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );
    ESTD_DEBUG("Fragment: %" PRIestr, ESTD_STRING_ARG(fragment_source));

    GLuint vertex_shader, fragment_shader;
    ESTD_BUBBLE_T(
        GalaResult,
        galaMakeShader(&vertex_shader, vertex_source, GL_VERTEX_SHADER),
        "Could not create vertex shader for program %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );
    ESTD_DEBUG("Created vertex shader for program %" PRIestr, ESTD_STRING_ARG(program.name));

    ESTD_BUBBLE_T(
        GalaResult,
        galaMakeShader(&fragment_shader, fragment_source, GL_FRAGMENT_SHADER),
        "Could not create fragment shader for program %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );
    ESTD_DEBUG("Created fragment shader for program %" PRIestr, ESTD_STRING_ARG(program.name));

    ESTD_BUBBLE_T(
        GalaResult,
        galaMakeProgram(&program.gl, vertex_shader, fragment_shader),
        "Could not create program %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );
    ESTD_DEBUG("Created program %" PRIestr, ESTD_STRING_ARG(program.name));

    *io_program = program;
    return GALA_SUCCESS;
}

// TODO: Add buffers
static GalaResult galaCreateVertexArray(GalaVertexArray* vertex_array) {
    GALA_GL(glGenVertexArrays(1, &vertex_array->gl), "Could not generate vertex array");
    GALA_GL(glBindVertexArray(vertex_array->gl), "Could not bind vertex array");
    for (size_t a = 0; a < vertex_array->attribute_count; a++) {
        GalaAttribute const* attr = &vertex_array->attributes[a];
        GALA_GL(glEnableVertexAttribArray(a), "Could not enable attribute %zu", a);
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
    GALA_GL(glBindVertexArray(0), "Could not unbind vertex array");
    return GALA_SUCCESS;
}

static GalaResult galaBindPipeline(GalaPipeline* pipeline) {
    GALA_GL(glUseProgram(pipeline->program->gl), "Could not use program");
    return GALA_SUCCESS;
}

static GalaResult galaBindVertexArray(GalaVertexArray* vertex_array) {
    GALA_GL(glBindVertexArray(vertex_array->gl), "Could not bind vertex array");
    return GALA_SUCCESS;
}

static GalaResult galaDrawArrays(int start, int count) {
    GALA_GL(glDrawArrays(GL_TRIANGLES, start, count), "Could not draw arrays");
    return GALA_SUCCESS;
}

GalaResult galaProcessCommand(GalaCommand* command) {
    EstdArena* arena;
    switch (command->type) {
        case GALA_COMMAND_TYPE_CREATE_PROGRAM:
            ESTD_BUBBLE_T(
                GalaResult,
                galaCreateProgram(command->create_program.program, &arena),
                "Could not create program"
            );
            break;
        case GALA_COMMAND_TYPE_CREATE_VERTEX_ARRAY:
            ESTD_BUBBLE_T(
                GalaResult,
                galaCreateVertexArray(command->create_vertex_array.vertex_array),
                "Could not create vertex array"
            );
            break;
        case GALA_COMMAND_TYPE_BIND_PIPELINE:
            ESTD_BUBBLE_T(GalaResult, galaBindPipeline(command->bind_pipeline.pipeline), "Could not bind pipeline");
            break;
        case GALA_COMMAND_TYPE_BIND_VERTEX_ARRAY:
            ESTD_BUBBLE_T(
                GalaResult,
                galaBindVertexArray(command->bind_vertex_array.vertex_array),
                "Could not bind vertex array"
            );
            break;
        case GALA_COMMAND_TYPE_DRAW_ARRAYS:
            ESTD_BUBBLE_T(
                GalaResult,
                galaDrawArrays(command->draw_arrays.start, command->draw_arrays.count),
                "Could not draw arrays"
            );
            break;
    }
    return GALA_SUCCESS;
}
