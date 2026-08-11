#define ESTD_ALL_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <engine/engine.h>
#include <estd/log.h>
#include <glad/glad.h>

#include <estd/arena.h>
#include <estd/str.h>
#include <estd/string_builder.h>

int main(int argc, char** argv) {
    glfwInit();
    ESTD_DEBUG("GLFW initialized");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "GALA-GLFW", NULL, NULL);
    if (!window) {
        return 1;
    }
    ESTD_DEBUG("GLFW window created");
    glfwMakeContextCurrent(window);
    ESTD_DEBUG("GLFW context set");
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    ESTD_DEBUG("GL loaded");

    EstdArena* arena = NULL;
    GalaEngine* engine;
    ESTD_OP_INT(
        galaCreateEngine(
            &engine,
            (GalaEngineConfig){
                .model_count = 1,
            },
            &arena
        ),
        "engine initialization"
    );

    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ESTD_BUBBLE_INT(galaUpdateEngine(engine, &arena), "updating engine");
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
