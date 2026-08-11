#include "engine.h"

#include <estd/result.h>
#include <estd/string_builder.h>
#include <glad/glad.h>
#include <stdlib.h>

#include "glresult.h"
#include "resource.h"

typedef enum GalaCommandType {
    GALA_COMMAND_TYPE_CREATE_PROGRAM
} GalaCommandType;

typedef struct GalaCommandCreateProgram {
    GalaProgram* program;
} GalaCommandCreateProgram;

typedef struct GalaCommand {
    GalaCommandType type;
    char const* file;
    char const* func;
    int line;
    union {
        GalaCommandCreateProgram create_program;
    };
} GalaCommand;

struct GalaEngine {
    size_t command_queue_count;
    size_t command_queue_capacity;
    GalaCommand* command_queue;

    size_t program_count;
    GalaProgram* programs;

    size_t vertex_array_count;
    GalaVertexArray* vertex_arrays;

    size_t buffer_count;
    GalaBuffer* buffers;

    size_t texture_count;
    GalaTexture* textures;
};

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

typedef enum GalaProgramName {
    GALA_PROGRAM_NAME_MESH,
    GALA_PROGRAM_NAME_QUAD
} GalaProgramName;

static GalaAttribute const mesh_attributes[] = {
    {
        .name = ESTD_LITERAL("aPosition"),
        .type = GALA_ATTRIBUTE_TYPE_FLOAT,
        .element_type = GALA_TYPE_F32,
        .count = 3,
    },
    {
        .name = ESTD_LITERAL("aTexcoord"),
        .type = GALA_ATTRIBUTE_TYPE_FLOAT,
        .element_type = GALA_TYPE_F32,
        .count = 2,
    },
    {
        .name = ESTD_LITERAL("aNormal"),
        .type = GALA_ATTRIBUTE_TYPE_NORMALIZED,
        .element_type = GALA_TYPE_U16,
        .count = 2,
    },
    {
        .name = ESTD_LITERAL("aTangent"),
        .type = GALA_ATTRIBUTE_TYPE_INTEGER,
        .element_type = GALA_TYPE_U16,
        .count = 2,
    },
    {
        .name = ESTD_LITERAL("aMeshId"),
        .type = GALA_ATTRIBUTE_TYPE_INTEGER,
        .element_type = GALA_TYPE_U32,
        .count = 1,
    },
};

static GalaProgram const programs[] = {
    [GALA_PROGRAM_NAME_MESH] =
        (GalaProgram){
            .name = ESTD_LITERAL("mesh"),
            .attribute_count = sizeof(mesh_attributes) / sizeof(mesh_attributes[0]),
            .attributes = mesh_attributes,
        },

    [GALA_PROGRAM_NAME_QUAD] = (GalaProgram){
        .name = ESTD_LITERAL("quad"),
        .attribute_count = 0,
        .attributes = NULL,
    },
};

static GalaResult galaPushCommand(GalaEngine* self, GalaCommand command) {
    if (self->command_queue_count == self->command_queue_capacity) {
        GalaCommand* new_queue =
            realloc(self->command_queue, self->command_queue_capacity * 2 * sizeof(self->command_queue[0]));
        if (new_queue == NULL) {
            ESTD_THROW(GALA_RESULT_OUT_OF_MEMORY, "Could not resize command queue");
        }
        self->command_queue = new_queue;
        self->command_queue_capacity *= 2;
    }
    self->command_queue[self->command_queue_count] = command;
    self->command_queue_count += 1;
    return GALA_SUCCESS;
}

#define galaPushCommand(self, ...) \
    galaPushCommand(self, (GalaCommand){.file = __FILE__, .func = __func__, .line = __LINE__, ##__VA_ARGS__})

GalaResult galaCreateEngine(GalaEngine** o_self, GalaEngineConfig config, EstdArena** allocator) {
    GalaEngine* self;
    ESTD_BUBBLE_T(GalaResult, estdArenaNew(&self, allocator), "Could not allocate engine");
    ESTD_DEBUG("Engine allocated");

    self->command_queue_count = 0;
    self->command_queue_capacity = 1024;
    self->command_queue = calloc(self->command_queue_capacity, sizeof(self->command_queue[0]));
    if (self->command_queue == NULL) {
        ESTD_THROW(GALA_RESULT_OUT_OF_MEMORY, "Could not allocate command queue");
    }

    self->program_count = sizeof(programs) / sizeof(programs[0]);
    ESTD_BUBBLE_T(
        GalaResult,
        estdArenaArray(&self->programs, allocator, self->program_count),
        "Could not allocate %zu programs",
        self->program_count
    );
    memcpy(self->programs, programs, sizeof(programs));
    for (size_t p = 0; p < self->program_count; p++) {
        galaPushCommand(
            self,
            .type = GALA_COMMAND_TYPE_CREATE_PROGRAM,
            .create_program = (GalaCommandCreateProgram){
                .program = &self->programs[p],
            },
        );
    }
    ESTD_DEBUG("Programs created");

    self->vertex_array_count = config.model_count;
    ESTD_BUBBLE_T(
        GalaResult,
        estdArenaArray(&self->vertex_arrays, allocator, self->vertex_array_count),
        "Could not allocate %zu vertex arrays",
        self->vertex_array_count
    );
    ESTD_DEBUG("VAOs created");

    self->buffer_count = config.model_count * 3;
    ESTD_BUBBLE_T(
        GalaResult,
        estdArenaArray(&self->buffers, allocator, self->buffer_count),
        "Could not allocate %zu buffers",
        self->buffer_count
    );
    ESTD_DEBUG("Buffers created");

    self->texture_count = config.model_count * 4;
    ESTD_BUBBLE_T(
        GalaResult,
        estdArenaArray(&self->textures, allocator, self->texture_count),
        "Could not allocate %zu textures",
        self->texture_count
    );
    ESTD_DEBUG("Textures created");

    *o_self = self;
    return GALA_SUCCESS;
}

GalaResult galaUpdateEngine(GalaEngine* self, EstdArena** allocator) {
    for (size_t c = 0; c < self->command_queue_count; c++) {
        GalaCommand const* command = &self->command_queue[c];
        switch (command->type) {
            case GALA_COMMAND_TYPE_CREATE_PROGRAM:
                ESTD_BUBBLE_T(
                    GalaResult,
                    galaCreateProgram(command->create_program.program, allocator),
                    "Could not process command (pushed from %s @ %s:%d)",
                    command->func,
                    command->file,
                    command->line
                );
                break;
        }
    }
    self->command_queue_count = 0;
    return GALA_SUCCESS;
}
