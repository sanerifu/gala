#ifndef __GALA_H__
#define __GALA_H__

#include <estd/result.h>

typedef enum GalaResult {
    GALA_SUCCESS = ESTD_SUCCESS,
    GALA_CANNOT_PUSH = ('G' << 24) | ('L' << 16)
} GalaResult;

typedef enum GalaEventType {
    GALA_EVENT_NONE = 0,
    GALA_EVENT_RESIZE
} GalaEventType;

typedef struct GalaEventResize {
    int width;
    int height;
} GalaEventResize;

typedef struct GalaEvent {
    GalaEventType type;
    union {
        GalaEventResize resize;
    } event;
} GalaEvent;

typedef struct GalaContext {
    int event_count;
    GalaEvent events[1024];
} GalaContext;

GalaResult galaInitContext(GalaContext* context);
void galaDestroyContext(GalaContext* context);
void galaDestroyContextWrapper(void* context);

GalaResult galaUpdate(GalaContext* context);
GalaResult galaRender(GalaContext* context);

GalaResult galaResize(GalaContext* context, int width, int height);

#endif
