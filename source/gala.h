#ifndef __GALA_H__
#define __GALA_H__

#include <estd/result.h>

typedef enum GalaResult {
    GALA_SUCCESS = ESTD_SUCCESS,
} GalaResult;

typedef struct GalaContext {
    int dummy;
} GalaContext;

GalaResult galaInitContext(GalaContext* context);
void galaDestroyContext(GalaContext* context);
void galaDestroyContextWrapper(void* context);

#endif
