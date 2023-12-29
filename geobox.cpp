#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define INIT_WINDOW_WIDTH 800
#define INIT_WINDOW_HEIGHT 600
#define WINDOW_TITLE "GeoBox"

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    // Initialize GLFW

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create Window

    GLFWwindow *window = glfwCreateWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set OpenGL viewport

    glViewport(0, 0, INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);

    // Set window resize callback

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Main loop

    while (!glfwWindowShouldClose(window))
    {
        // Poll events
        glfwPollEvents();

        // Update state

        // Render to framebuffer

        // Swap/Present framebuffer to screen
        glfwSwapBuffers(window);

        // Delay to control FPS
    }

    glfwTerminate();
    return 0;
}