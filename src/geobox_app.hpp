#pragma once

#include <iosfwd> // forward declaration of std::ifstream, ...
#include <string>

#include <GLFW/glfw3.h>

class GeoBox_App
{
public:
    GeoBox_App();
    void main_loop();

private:
    GLFWwindow *m_window = nullptr;

    void init_glfw();
    void init_glad() const;
    void init_imgui();
    void init_gl_viewport() const;
    void init_glfw_callbacks();

    void window_refresh_callback(GLFWwindow *window);
    void framebuffer_size_callback(const GLFWwindow *window, int width, int height) const;

    // main loop internals
    void process_input();
    void render();
    void shutdown() const;

    // Dialogs
    void on_load_stl_dialog_ok(const std::string &file_path);
};