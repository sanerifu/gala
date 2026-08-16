#include "engine.h"

#include <estd/result.h>
#include <estd/string_builder.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "commands.h"
#include "glresult.h"
#include "resource.h"

ESTD_RESULT_DATA(___GALA_RESULTS);

GALA_STRUCT(GalaEngine) {
    size_t command_queue_count;
    size_t command_queue_capacity;
    GalaCommand* command_queue;

    size_t program_count;
    GalaProgram* programs;

    size_t pipeline_count;
    GalaPipeline* pipelines;

    size_t vertex_array_count;
    GalaVertexArray* vertex_arrays;

    size_t buffer_count;
    GalaBuffer* buffers;

    size_t texture_count;
    GalaTexture* textures;
};

GALA_ENUM(GalaProgramName){GALA_PROGRAM_NAME_MESH, GALA_PROGRAM_NAME_QUAD};

GALA_ENUM(GalaPipelineName){GALA_PIPELINE_NAME_MESH, GALA_PIPELINE_NAME_QUAD};

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

static GalaTextureUnit const mesh_texture_units[] = {
    {
        .name = ESTD_LITERAL("tAlbedo"),
        .type = GALA_TEXTURE_TYPE_ARRAY_2D,
        .format = GALA_TEXTURE_FORMAT_RGBA8,
    },
    {
        .name = ESTD_LITERAL("tNormal"),
        .type = GALA_TEXTURE_TYPE_ARRAY_2D,
        .format = GALA_TEXTURE_FORMAT_RGB8,
    },
    {
        .name = ESTD_LITERAL("tOrm"),
        .type = GALA_TEXTURE_TYPE_ARRAY_2D,
        .format = GALA_TEXTURE_FORMAT_RGB8,
    },
    {
        .name = ESTD_LITERAL("tEmissive"),
        .type = GALA_TEXTURE_TYPE_ARRAY_2D,
        .format = GALA_TEXTURE_FORMAT_RGB8,
    }
};

static GalaTextureUnit const quad_texture_units[] = {
    {
        .name = ESTD_LITERAL("tColor"),
        .type = GALA_TEXTURE_TYPE_2D,
        .format = GALA_TEXTURE_FORMAT_RGBA8,
    },
};

static GalaProgram const programs[] = {
    [GALA_PROGRAM_NAME_MESH] =
        (GalaProgram){
            .name = ESTD_LITERAL("mesh"),

            .attribute_count = sizeof(mesh_attributes) / sizeof(mesh_attributes[0]),
            .attributes = mesh_attributes,

            .texture_unit_count = sizeof(mesh_texture_units) / sizeof(mesh_texture_units[0]),
            .texture_units = mesh_texture_units,
        },

    [GALA_PROGRAM_NAME_QUAD] = (GalaProgram){
        .name = ESTD_LITERAL("quad"),

        .attribute_count = 0,
        .attributes = NULL,

        .texture_unit_count = sizeof(quad_texture_units) / sizeof(quad_texture_units[0]),
        .texture_units = quad_texture_units,
    },
};

static GalaResult galaPushCommand(GalaEngine* self, GalaCommand command) {
    if (self->command_queue_count == self->command_queue_capacity) {
        GalaCommand* new_queue =
            realloc(self->command_queue, self->command_queue_capacity * 2 * sizeof(self->command_queue[0]));
        if (new_queue == NULL) {
            ESTD_THROW(GALA_RESULT_OUT_OF_MEMORY, "command queue resize");
        }
        self->command_queue = new_queue;
        self->command_queue_capacity *= 2;
    }
    self->command_queue[self->command_queue_count] = command;
    self->command_queue_count += 1;
    return GALA_SUCCESS;
}

GALA_STRUCT(GalaMeshVertex) {
    float pos[3];
    float tex[2];
    uint16_t nrm[2];
    uint16_t tgn[2];
    uint32_t mid;
};

static GalaMeshVertex const triangle_vertices[] = {
    (GalaMeshVertex){
        .pos = {-1.0f, -1.0f, 0.0f},
        .tex = {0.0f, 0.0f},
    },
    (GalaMeshVertex){
        .pos = {3.0f, -1.0f, 0.0f},
        .tex = {2.0f, 0.0f},
    },
    (GalaMeshVertex){
        .pos = {-1.0f, 3.0f, 0.0f},
        .tex = {0.0f, 2.0f},
    },
};

#define galaPushCommand(self, ...) \
    galaPushCommand(self, (GalaCommand){.file = __FILE__, .func = __func__, .line = __LINE__, ##__VA_ARGS__})

GalaResult galaCreateEngine(GalaEngine** o_self, GalaEngineConfig config, EstdArena** allocator) {
    GalaEngine* self;
    ESTD_OP(estdArenaNew(&self, allocator), "engine allocation");

    self->command_queue_count = 0;
    self->command_queue_capacity = 1024;
    self->command_queue = calloc(self->command_queue_capacity, sizeof(self->command_queue[0]));
    if (self->command_queue == NULL) {
        ESTD_THROW(GALA_RESULT_OUT_OF_MEMORY, "command queue allocation");
    }

    self->program_count = sizeof(programs) / sizeof(programs[0]);
    ESTD_OP(
        estdArenaArray(&self->programs, allocator, self->program_count),
        "allocating %zu programs",
        self->program_count
    );
    memcpy(self->programs, programs, sizeof(programs));
    for (size_t p = 0; p < self->program_count; p++) {
        ESTD_OP(
            galaPushCommand(
                self,
                .type = GALA_COMMAND_CREATE_PROGRAM,
                .create_program =
                    {
                        .program = &self->programs[p],
                    },
            ),
            "pushing creation of program %" PRIestr,
            ESTD_STRING_ARG(self->programs[p].name)
        );
    }

    self->pipeline_count = 2;
    ESTD_OP(
        estdArenaArray(&self->pipelines, allocator, self->pipeline_count),
        "allocating %zu pipelines",
        self->pipeline_count
    );
    self->pipelines[GALA_PIPELINE_NAME_MESH] = (GalaPipeline){
        .program = &self->programs[GALA_PROGRAM_NAME_MESH],
    };
    self->pipelines[GALA_PIPELINE_NAME_QUAD] = (GalaPipeline){
        .program = &self->programs[GALA_PROGRAM_NAME_QUAD],
    };

    self->buffer_count = config.model_count * 3;
    ESTD_OP(
        estdArenaArray(&self->buffers, allocator, self->buffer_count),
        "allocating %zu buffers",
        self->buffer_count
    );
    self->buffers[0] = (GalaBuffer){
        .name = ESTD_LITERAL("TestVbo"),
        .usage = GALA_BUFFER_USAGE_STATIC_DRAW,
        .type = GALA_BUFFER_TYPE_ARRAY,
        .data = (uint8_t*)triangle_vertices,
        .size = sizeof(triangle_vertices),
    };
    ESTD_OP(
        galaPushCommand(self, .type = GALA_COMMAND_CREATE_BUFFER, .create_buffer = {.buffer = &self->buffers[0]}),
        "pushing triangle buffer data"
    );

    self->vertex_array_count = 2 + config.model_count;
    ESTD_OP(
        estdArenaArray(&self->vertex_arrays, allocator, self->vertex_array_count),
        "allocating %zu vertex arrays",
        self->vertex_array_count
    );
    self->vertex_arrays[0] = (GalaVertexArray){
        .name = ESTD_LITERAL("TestVao"),
        .attribute_count = 0,
        .attributes = NULL,
    };
    self->vertex_arrays[1] = (GalaVertexArray){
        .name = ESTD_LITERAL("AttrTestVao"),
        .attribute_count = sizeof(mesh_attributes) / sizeof(mesh_attributes[0]),
        .attributes = mesh_attributes,
    };
    ESTD_OP(
        galaPushCommand(
            self,
            .type = GALA_COMMAND_CREATE_VERTEX_ARRAY,
            .create_vertex_array = {.vertex_array = &self->vertex_arrays[0]}
        ),
        "pushing empty vertex array creation command"
    );
    ESTD_OP(
        galaPushCommand(
            self,
            .type = GALA_COMMAND_CREATE_VERTEX_ARRAY,
            .create_vertex_array = {.vertex_array = &self->vertex_arrays[1], .vertex_buffer = &self->buffers[0]}
        ),
        "pushing mesh vertex array creation command"
    );

    self->texture_count = 1 + config.model_count * 4;
    ESTD_OP(
        estdArenaArray(&self->textures, allocator, self->texture_count),
        "allocating %zu textures",
        self->texture_count
    );
    int width, height;
    uint8_t* image_data = stbi_load("assets/test.jpg", &width, &height, NULL, 4);
    self->textures[0] = (GalaTexture){
        .name = ESTD_LITERAL("TestTexture"),
        .type = GALA_TEXTURE_TYPE_2D,
        .format = GALA_TEXTURE_FORMAT_RGBA8,
        .mipmap_count = 1,
        .width = width,
        .height = height,
        .depth = 1,
        .wrap_s = GALA_TEXTURE_WRAP_CLAMP_TO_EDGE,
        .wrap_t = GALA_TEXTURE_WRAP_CLAMP_TO_EDGE,
        .wrap_r = GALA_TEXTURE_WRAP_CLAMP_TO_EDGE,
        .minification = GALA_TEXTURE_MIN_LINEAR,
        .magnification = GALA_TEXTURE_MAG_LINEAR,
        .data = image_data,
    };
    ESTD_OP(
        galaPushCommand(self, .type = GALA_COMMAND_CREATE_TEXTURE, .create_texture = {.texture = &self->textures[0]}),
        "pushing texture creation command"
    );

    *o_self = self;
    return GALA_SUCCESS;
}

GalaResult galaUpdateEngine(GalaEngine* self, EstdArena** allocator) {
    ESTD_BUBBLE(
        galaPushCommand(
            self,
            .type = GALA_COMMAND_BIND_PIPELINE,
            .bind_pipeline = {.pipeline = &self->pipelines[GALA_PIPELINE_NAME_QUAD]}
        ),
        "pushing bind pipeline command"
    );

    ESTD_BUBBLE(
        galaPushCommand(
            self,
            .type = GALA_COMMAND_BIND_TEXTURE,
            .bind_texture = {.unit = 0, .texture = &self->textures[0]}
        ),
        "pushing bind texture command"
    );

    ESTD_BUBBLE(
        galaPushCommand(
            self,
            .type = GALA_COMMAND_BIND_VERTEX_ARRAY,
            .bind_vertex_array = {.vertex_array = &self->vertex_arrays[0]}
        ),
        "pushing bind vertex array command"
    );

    ESTD_BUBBLE(
        galaPushCommand(self, .type = GALA_COMMAND_DRAW_ARRAYS, .draw_arrays = {.start = 0, .count = 3}),
        "pushing draw triangle command"
    );

    for (size_t c = 0; c < self->command_queue_count; c++) {
        ESTD_BUBBLE(
            galaProcessCommand(&self->command_queue[c]),
            "processing command (pushed in %s @ %s:%d)",
            self->command_queue[c].func,
            self->command_queue[c].file,
            self->command_queue[c].line
        );
    }
    self->command_queue_count = 0;
    return GALA_SUCCESS;
}
