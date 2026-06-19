#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <estd/arena.h>
#include <estd/result.h>
#include <glad/glad.h>

#include "gala/gala.h"

enum GalaWindowResult {
    GALA_WINDOW_INITILIZATION_FAIL
};

void windowSizeCallback(GLFWwindow* window, int width, int height) {
    GalaContext* engine = (GalaContext*)glfwGetWindowUserPointer(window);
    if(engine == NULL) {
        return;
    }
    ESTD_ASSERT_PANIC(galaResize(engine, width, height) == GALA_SUCCESS, "Could not resize engine");
}

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test app", NULL, NULL);
    if (window == NULL) {
        ESTD_THROW(GALA_WINDOW_INITILIZATION_FAIL, "Could not initialize window");
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    GalaContext ESTD_CLEAN(galaDestroyContext) engine = {0};
    ESTD_BUBBLE(galaInitContext(&engine), "Could not initialize engine context");

    glfwSetWindowUserPointer(window, &engine);
    glfwSetWindowSizeCallback(window, &windowSizeCallback);

    while (!glfwWindowShouldClose(window)) {
        galaUpdate(&engine);
        galaRender(&engine);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
