#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <estd/arena.h>
#include <estd/result.h>
#include <glad/glad.h>

#include "gala.h"

enum GalaWindowResult {
    GALA_WINDOW_INITILIZATION_FAIL
};

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

    while (!glfwWindowShouldClose(window)) {
        galaUpdate(&engine);
        galaRender(&engine);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
