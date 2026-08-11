#include "engine.h"

#include <estd/result.h>
#include <estd/string_builder.h>
#include <glad/glad.h>
#include <stdlib.h>

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
    ESTD_BUBBLE(estdArenaNew(&self, allocator), "Could not allocate engine");
    ESTD_DEBUG("Engine allocated");

    self->command_queue_count = 0;
    self->command_queue_capacity = 1024;
    self->command_queue = calloc(self->command_queue_capacity, sizeof(self->command_queue[0]));
    if (self->command_queue == NULL) {
        ESTD_THROW(GALA_RESULT_OUT_OF_MEMORY, "Could not allocate command queue");
    }

    self->program_count = sizeof(programs) / sizeof(programs[0]);
    ESTD_BUBBLE(
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

    self->pipeline_count = 2;
    ESTD_BUBBLE(
        estdArenaArray(&self->pipelines, allocator, self->pipeline_count),
        "Could not allocate %zu pipelines",
        self->pipeline_count
    );
    self->pipelines[GALA_PIPELINE_NAME_MESH] = (GalaPipeline){
        .program = &self->programs[GALA_PROGRAM_NAME_MESH],
    };
    self->pipelines[GALA_PIPELINE_NAME_QUAD] = (GalaPipeline){
        .program = &self->programs[GALA_PROGRAM_NAME_QUAD],
    };

    self->vertex_array_count = 1 + config.model_count;
    ESTD_BUBBLE(
        estdArenaArray(&self->vertex_arrays, allocator, self->vertex_array_count),
        "Could not allocate %zu vertex arrays",
        self->vertex_array_count
    );
    self->vertex_arrays[0] = (GalaVertexArray){
        .name = ESTD_LITERAL("TestVao"),
        .attribute_count = 0,
        .attributes = NULL,
    };
    galaPushCommand(
        self,
        .type = GALA_COMMAND_TYPE_CREATE_VERTEX_ARRAY,
        .create_vertex_array = {.vertex_array = &self->vertex_arrays[0]}
    );
    ESTD_DEBUG("VAOs created");

    self->buffer_count = config.model_count * 3;
    ESTD_BUBBLE(
        estdArenaArray(&self->buffers, allocator, self->buffer_count),
        "Could not allocate %zu buffers",
        self->buffer_count
    );
    ESTD_DEBUG("Buffers created");

    self->texture_count = config.model_count * 4;
    ESTD_BUBBLE(
        estdArenaArray(&self->textures, allocator, self->texture_count),
        "Could not allocate %zu textures",
        self->texture_count
    );
    ESTD_DEBUG("Textures created");

    *o_self = self;
    return GALA_SUCCESS;
}

GalaResult galaUpdateEngine(GalaEngine* self, EstdArena** allocator) {
    galaPushCommand(
        self,
        .type = GALA_COMMAND_TYPE_BIND_PIPELINE,
        .bind_pipeline = {.pipeline = &self->pipelines[GALA_PIPELINE_NAME_QUAD]}
    );

    galaPushCommand(
        self,
        .type = GALA_COMMAND_TYPE_BIND_VERTEX_ARRAY,
        .bind_vertex_array = {.vertex_array = &self->vertex_arrays[0]}
    );

    galaPushCommand(self, .type = GALA_COMMAND_TYPE_DRAW_ARRAYS, .draw_arrays = {.start = 0, .count = 3});

    for (size_t c = 0; c < self->command_queue_count; c++) {
        ESTD_BUBBLE(
            galaProcessCommand(&self->command_queue[c]),
            "Could not process command (pushed in %s @ %s:%d)",
            self->command_queue[c].func,
            self->command_queue[c].file,
            self->command_queue[c].line
        );
    }
    self->command_queue_count = 0;
    return GALA_SUCCESS;
}
