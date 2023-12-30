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

    /* TODO: get rid of this monstrosity, 
       store meshes in a vector or something,
       and load shaders in initialization
    */
    unsigned int m_VAO;
    unsigned int m_shader_program;
    unsigned int m_num_vertices;
    bool m_is_mesh_loaded = false;

    void init_glfw();
    void init_glad() const;
    void init_imgui();
    void init_gl_viewport() const;
    void init_glfw_callbacks();

    void window_refresh_callback(GLFWwindow *window);
    void framebuffer_size_callback(const GLFWwindow *window, int width, int height) const;

    // Main-loop internals
    void process_input();
    void render();
    void shutdown() const;

    // Dialogs
    void on_load_stl_dialog_ok(const std::string &file_path);
};