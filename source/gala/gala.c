#include "gala.h"

#include <glad/glad.h>

GalaResult galaInitContext(GalaContext* context) {
    return GALA_SUCCESS;
}

void galaDestroyContext(GalaContext* context) {}

void galaDestroyContextWrapper(void* context) {
    GalaContext* ctx = (GalaContext*)context;
    galaDestroyContext(ctx);
}

GalaResult galaUpdate(GalaContext* context) {
    ESTD_DEBUG("UPDATE");
    for (int i = 0; i < context->event_count; i++) {
        switch (context->events[i].type) {
            case GALA_EVENT_NONE:
                break;
            case GALA_EVENT_RESIZE:
                glViewport(0, 0, context->events[i].event.resize.width, context->events[i].event.resize.height);
                break;
        }
        ESTD_DEBUG("Handled event %d", i);
    }
    context->event_count = 0;
    return GALA_SUCCESS;
}

GalaResult galaRender(GalaContext* context) {
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return GALA_SUCCESS;
}

GalaResult galaResize(GalaContext* context, int width, int height) {
    GalaEvent event = {
        .type = GALA_EVENT_RESIZE,
        .event.resize = {
            .width = width,
            .height = height,
        },
    };

    ESTD_ASSERT(
        GALA_CANNOT_PUSH,
        context->event_count < (sizeof(context->events) / sizeof(context->events[0])),
        "Not enough space"
    );
    ESTD_DEBUG("%d", context->event_count);
    context->events[context->event_count] = event;
    context->event_count += 1;

    return GALA_SUCCESS;
}
