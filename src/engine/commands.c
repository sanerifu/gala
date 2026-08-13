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

static size_t galaGetAttributeSize(GalaAttribute const* attr) {
    size_t element_size;
    switch (attr->element_type) {
        case GALA_TYPE_S8:
            element_size = 1;
            break;
        case GALA_TYPE_U8:
            element_size = 1;
            break;
        case GALA_TYPE_S16:
            element_size = 2;
            break;
        case GALA_TYPE_U16:
            element_size = 2;
            break;
        case GALA_TYPE_F16:
            element_size = 2;
            break;
        case GALA_TYPE_S32:
            element_size = 4;
            break;
        case GALA_TYPE_U32:
            element_size = 4;
            break;
        case GALA_TYPE_F32:
            element_size = 4;
            break;
        default:
            element_size = 0;
    }
    return element_size * attr->count;
}

static EstdString galaGetTextureFormatPrefix(GalaTextureFormat format) {
    switch (format) {
        case GALA_TEXTURE_FORMAT_R8I:
        case GALA_TEXTURE_FORMAT_R16I:
        case GALA_TEXTURE_FORMAT_R32I:
        case GALA_TEXTURE_FORMAT_RG8I:
        case GALA_TEXTURE_FORMAT_RG16I:
        case GALA_TEXTURE_FORMAT_RG32I:
        case GALA_TEXTURE_FORMAT_RGBA32I:
        case GALA_TEXTURE_FORMAT_RGB32I:
        case GALA_TEXTURE_FORMAT_RGBA16I:
        case GALA_TEXTURE_FORMAT_RGB16I:
        case GALA_TEXTURE_FORMAT_RGBA8I:
        case GALA_TEXTURE_FORMAT_RGB8I:
            return ESTD_LITERAL("i");

        case GALA_TEXTURE_FORMAT_R8UI:
        case GALA_TEXTURE_FORMAT_R16UI:
        case GALA_TEXTURE_FORMAT_R32UI:
        case GALA_TEXTURE_FORMAT_RG8UI:
        case GALA_TEXTURE_FORMAT_RG16UI:
        case GALA_TEXTURE_FORMAT_RG32UI:
        case GALA_TEXTURE_FORMAT_RGBA32UI:
        case GALA_TEXTURE_FORMAT_RGB32UI:
        case GALA_TEXTURE_FORMAT_RGBA16UI:
        case GALA_TEXTURE_FORMAT_RGB16UI:
        case GALA_TEXTURE_FORMAT_RGBA8UI:
        case GALA_TEXTURE_FORMAT_RGB8UI:
            return ESTD_LITERAL("u");

        case GALA_TEXTURE_FORMAT_R8:
        case GALA_TEXTURE_FORMAT_R16:
        case GALA_TEXTURE_FORMAT_RG8:
        case GALA_TEXTURE_FORMAT_RG16:
        case GALA_TEXTURE_FORMAT_R16F:
        case GALA_TEXTURE_FORMAT_R32F:
        case GALA_TEXTURE_FORMAT_RG16F:
        case GALA_TEXTURE_FORMAT_RG32F:
        case GALA_TEXTURE_FORMAT_R3_G3_B2:
        case GALA_TEXTURE_FORMAT_RGB4:
        case GALA_TEXTURE_FORMAT_RGB5:
        case GALA_TEXTURE_FORMAT_RGB8:
        case GALA_TEXTURE_FORMAT_RGB10:
        case GALA_TEXTURE_FORMAT_RGB12:
        case GALA_TEXTURE_FORMAT_RGB16:
        case GALA_TEXTURE_FORMAT_RGBA2:
        case GALA_TEXTURE_FORMAT_RGBA4:
        case GALA_TEXTURE_FORMAT_RGB5_A1:
        case GALA_TEXTURE_FORMAT_RGBA8:
        case GALA_TEXTURE_FORMAT_RGB10_A2:
        case GALA_TEXTURE_FORMAT_RGBA12:
        case GALA_TEXTURE_FORMAT_RGBA16:
        case GALA_TEXTURE_FORMAT_SRGB8:
        case GALA_TEXTURE_FORMAT_SRGBA8:
        case GALA_TEXTURE_FORMAT_RGB9_E5:
        case GALA_TEXTURE_FORMAT_R11F_G11F_B10F:
            return ESTD_LITERAL("");
    }
    return ESTD_LITERAL("");
}

static EstdString galaGetTextureTypeName(GalaTextureType type) {
    switch (type) {
        case GALA_TEXTURE_TYPE_2D:
            return ESTD_LITERAL("sampler2D");
        case GALA_TEXTURE_TYPE_ARRAY_2D:
            return ESTD_LITERAL("sampler2DArray");
        case GALA_TEXTURE_TYPE_CUBEMAP:
            return ESTD_LITERAL("samplerCubemap");
    }
    return ESTD_LITERAL("");
}

static GalaResult galaGetTextureUnitText(EstdString* o_ret, GalaTextureUnit const* unit, EstdArena** allocator) {
    ESTD_OP(
        estdStringFormat(
            o_ret,
            allocator,
            "uniform %" PRIestr "%" PRIestr " %" PRIestr ";",
            ESTD_STRING_ARG(galaGetTextureFormatPrefix(unit->format)),
            ESTD_STRING_ARG(galaGetTextureTypeName(unit->type)),
            ESTD_STRING_ARG(unit->name)
        ),
        "creating uniform string for texture unit %" PRIestr,
        ESTD_STRING_ARG(unit->name)
    );
    return GALA_SUCCESS;
}

static GalaResult galaMakeShader(GLuint* o_shader, EstdString source, GLenum type) {
    ESTD_INFO("source: %" PRIestr, ESTD_STRING_ARG(source));
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

    for (size_t t = 0; t < program.texture_unit_count; t++) {
        GalaTextureUnit const* tex = &program.texture_units[t];
        EstdString declaration;
        ESTD_OP(
            galaGetTextureUnitText(&declaration, tex, &arena),
            "creating texture unit text for %" PRIestr,
            ESTD_STRING_ARG(tex->name)
        );
        ESTD_OP(
            estdStringBuilderAppendf(&vertex_source_builder, &arena, "%" PRIestr "\n", ESTD_STRING_ARG(declaration)),
            "prepending texture unit %" PRIestr,
            ESTD_STRING_ARG(tex->name)
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
        ),
        "prepending prelude to fragment shader %" PRIestr,
        ESTD_STRING_ARG(program.name)
    );

    for (size_t t = 0; t < program.texture_unit_count; t++) {
        GalaTextureUnit const* tex = &program.texture_units[t];
        EstdString declaration;
        ESTD_OP(
            galaGetTextureUnitText(&declaration, tex, &arena),
            "creating texture unit text for %" PRIestr,
            ESTD_STRING_ARG(tex->name)
        );
        ESTD_OP(
            estdStringBuilderAppendf(&fragment_source_builder, &arena, "%" PRIestr "\n", ESTD_STRING_ARG(declaration)),
            "prepending texture unit %" PRIestr,
            ESTD_STRING_ARG(tex->name)
        );
    }

    ESTD_OP(
        estdStringBuilderAppendf(
            &fragment_source_builder,
            &arena,
            "#line 20001\n"
            "%" PRIestr,
            ESTD_STRING_ARG(program.source)
        ),
        "appending source %" PRIestr " to vertex shader source",
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

    GALA_GLOP(glUseProgram(program.gl), "binding program");

    for (size_t t = 0; t < program.texture_unit_count; t++) {
        GalaTextureUnit const* unit = &program.texture_units[t];
        EstdString name;
        ESTD_OP(
            estdStringDuplicate(&name, unit->name, &arena),
            "duplicating texture unit name %" PRIestr,
            ESTD_STRING_ARG(unit->name)
        );
        GLint uniform_location;
        GALA_GLOP(
            uniform_location = glGetUniformLocation(program.gl, name.data),
            "getting uniform location for texture unit %" PRIestr,
            ESTD_STRING_ARG(unit->name)
        );
        GALA_GLOP(glad_glUniform1i(uniform_location, t), "setting texture unit %" PRIestr, ESTD_STRING_ARG(unit->name));
    }

    *io_program = program;
    return GALA_SUCCESS;
}

static GalaResult galaCreateBuffer(GalaBuffer* io_buffer) {
    GalaBuffer buffer = *io_buffer;

    GALA_GLOP(glGenBuffers(1, &buffer.gl), "generating buffer %" PRIestr, ESTD_STRING_ARG(buffer.name));
    GALA_GLOP(glBindBuffer((GLenum)buffer.type, buffer.gl), "binding buffer %" PRIestr, ESTD_STRING_ARG(buffer.name));
    GALA_GLOP(
        glBufferData((GLenum)buffer.type, buffer.size, buffer.data, (GLenum)buffer.usage),
        "allocating buffer %" PRIestr,
        ESTD_STRING_ARG(buffer.name)
    );

    *io_buffer = buffer;
    return GALA_SUCCESS;
}

static GalaResult
galaCreateVertexArray(GalaVertexArray* vertex_array, GalaBuffer* vertex_buffer, GalaBuffer* instance_buffer) {
    if (vertex_array->attributes != 0) {
        ESTD_ASSERT(
            GALA_RESULT_NON_MATCHING_BUFFER_TYPES,
            vertex_buffer->type == GALA_BUFFER_TYPE_ARRAY,
            "Vertex buffer must be an array buffer"
        );
    }
    GALA_GLOP(glGenVertexArrays(1, &vertex_array->gl), "generating vertex array");
    GALA_GLOP(glBindVertexArray(vertex_array->gl), "binding vertex array");
    if (vertex_array->attributes != 0) {
        GALA_GLOP(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->gl), "binding vertex buffer");
    }
    size_t offset = 0;
    size_t stride = 0;
    for (size_t a = 0; a < vertex_array->attribute_count; a++) {
        stride += galaGetAttributeSize(&vertex_array->attributes[a]);
    }
    for (size_t a = 0; a < vertex_array->attribute_count; a++) {
        GalaAttribute const* attr = &vertex_array->attributes[a];
        GALA_GLOP(
            glEnableVertexAttribArray(a),
            "enabling attribute %zu named %" PRIestr,
            a,
            ESTD_STRING_ARG(attr->name)
        );
        GLboolean normalized = GL_FALSE;
        switch (attr->type) {
            case GALA_ATTRIBUTE_TYPE_INTEGER:
                GALA_GLOP(
                    glVertexAttribIPointer(a, attr->count, attr->element_type, stride, (void*)offset),
                    "setting integer attribute %" PRIestr " at offset %zu with stride %zu",
                    ESTD_STRING_ARG(attr->name),
                    offset,
                    stride
                );
                break;
            case GALA_ATTRIBUTE_TYPE_NORMALIZED:
                normalized = GL_TRUE;
            case GALA_ATTRIBUTE_TYPE_FLOAT:
                GALA_GLOP(
                    glVertexAttribPointer(a, attr->count, attr->element_type, normalized, stride, (void*)offset),
                    "setting float attribute %" PRIestr " at offset %zu with stride %zu",
                    ESTD_STRING_ARG(attr->name),
                    offset,
                    stride
                );
                break;
        }
        offset += galaGetAttributeSize(attr);
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
    EstdArena* arena = NULL;
    switch (command->type) {
        case GALA_COMMAND_CREATE_PROGRAM:
            ESTD_BUBBLE(galaCreateProgram(command->create_program.program, &arena), "creating program");
            break;
        case GALA_COMMAND_CREATE_BUFFER:
            ESTD_BUBBLE(galaCreateBuffer(command->create_buffer.buffer), "creating buffer");
            break;
        case GALA_COMMAND_CREATE_VERTEX_ARRAY:
            ESTD_BUBBLE(
                galaCreateVertexArray(
                    command->create_vertex_array.vertex_array,
                    command->create_vertex_array.vertex_buffer,
                    command->create_vertex_array.instance_buffer
                ),
                "creating vertex array %" PRIestr,
                ESTD_STRING_ARG(command->create_vertex_array.vertex_array->name)
            );
            break;
        case GALA_COMMAND_BIND_PIPELINE:
            ESTD_BUBBLE(galaBindPipeline(command->bind_pipeline.pipeline), "binding pipeline");
            break;
        case GALA_COMMAND_BIND_VERTEX_ARRAY:
            ESTD_BUBBLE(galaBindVertexArray(command->bind_vertex_array.vertex_array), "binding vertex array");
            break;
        case GALA_COMMAND_DRAW_ARRAYS:
            ESTD_BUBBLE(galaDrawArrays(command->draw_arrays.start, command->draw_arrays.count), "drawing arrays");
            break;
    }
    return GALA_SUCCESS;
}
