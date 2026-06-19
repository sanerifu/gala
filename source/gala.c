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

GalaResult galaUpdate(GalaContext* context) {}

GalaResult galaRender(GalaContext* context) {
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
