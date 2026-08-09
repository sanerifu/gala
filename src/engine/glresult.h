#ifndef __GALA_GLRESULT_H__
#define __GALA_GLRESULT_H__

#include <estd/str.h>
#include <glad/glad.h>

#include "engine.h"

#ifndef NDEBUG
#define GALA_GL(expr, fmt, ...)                                                    \
    do {                                                                           \
        expr;                                                                      \
        GLenum result;                                                             \
        GLenum last_result = GL_NO_ERROR;                                          \
        EstdString error_name;                                                     \
        while ((result = glGetError()) != GL_NO_ERROR) {                           \
            last_result = result;                                                  \
            switch (result) {                                                      \
                case GL_INVALID_ENUM:                                              \
                    error_name = ESTD_LITERAL("GL_INVALID_ENUM");                  \
                    break;                                                         \
                case GL_INVALID_VALUE:                                             \
                    error_name = ESTD_LITERAL("GL_INVALID_VALUE");                 \
                    break;                                                         \
                case GL_INVALID_OPERATION:                                         \
                    error_name = ESTD_LITERAL("GL_INVALID_OPERATION");             \
                    break;                                                         \
                case GL_INVALID_FRAMEBUFFER_OPERATION:                             \
                    error_name = ESTD_LITERAL("GL_INVALID_FRAMEBUFFER_OPERATION"); \
                    break;                                                         \
                case GL_OUT_OF_MEMORY:                                             \
                    error_name = ESTD_LITERAL("GL_OUT_OF_MEMORY");                 \
                    break;                                                         \
                default:                                                           \
                    error_name = ESTD_LITERAL("GL_UNKNOWN_ERROR");                 \
                    break;                                                         \
            }                                                                      \
            ESTD_ERROR("OpenGL error %" PRIestr, ESTD_STRING_ARG(error_name));     \
        }                                                                          \
        if (last_result != GL_NO_ERROR) {                                          \
            ESTD_TRACE(fmt, ##__VA_ARGS__);                                        \
            return GALA_RESULT_OPENGL_ERROR;                                       \
        }                                                                          \
    } while (0)

#else
#define GALA_GL(expr, fmt, ...) \
    do {                        \
        expr;                   \
    } while (0)
#endif

#endif
