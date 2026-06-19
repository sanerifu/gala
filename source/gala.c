#include "gala.h"

GalaResult galaInitContext(GalaContext* context) {
    return GALA_SUCCESS;
}

void galaDestroyContext(GalaContext* context) {}

void galaDestroyContextWrapper(void* context) {
    GalaContext* ctx = (GalaContext*)context;
    galaDestroyContext(ctx);
}
