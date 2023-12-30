#include <iostream>
#include <string>
#include <fstream>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "ImGuiFileDialog.h"

#include "vec3f.hpp"

constexpr int INIT_WINDOW_WIDTH = 800;
constexpr int INIT_WINDOW_HEIGHT = 600;
constexpr const char *WINDOW_TITLE = "GeoBox";

constexpr ImVec2 INITIAL_IMGUI_WINDOW_SIZE(550, 680);

constexpr const char *LOAD_STL_DIALOG_KEY = "Load_STL_Dialog_Key";
constexpr const char *LOAD_STL_BUTTON_DIALOG_TITLE = "Load .stl";

constexpr size_t BINARY_STL_HEADER_SIZE = 80;

struct Triangle
{
    Vec3f normal;
    std::array<Vec3f, 3> vertices;
};

void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

static size_t calc_file_size(std::ifstream &ifs)
{
    auto original_pos = ifs.tellg();
    ifs.seekg(0, std::ifstream::end);
    auto end = ifs.tellg();
    ifs.seekg(0, std::ifstream::beg);
    size_t file_size = end - ifs.tellg();
    ifs.seekg(original_pos, std::ifstream::beg);
    return file_size;
}

static size_t calc_expected_stl_mesh_file_binary_size(uint32_t num_triangles)
{
    static_assert(sizeof(Triangle) == sizeof(float[4][3]), "Incorrect size for Triangle struct"); // Normal + 3 vertices = 4 * 3 floats
    return BINARY_STL_HEADER_SIZE + sizeof(uint32_t) + num_triangles * (sizeof(Triangle) + sizeof(uint16_t));
}

void read_stl_mesh_file_binary(std::ifstream &ifs, uint32_t num_triangles)
{
    Triangle t;
    for (uint32_t i = 0; i < num_triangles; i++)
    {
        ifs.read((char *)&t, sizeof(Triangle));
        // Skip "attribute byte count"
        ifs.seekg(sizeof(uint16_t), std::ifstream::cur);
    }
}

void read_stl_mesh_file_ascii(std::ifstream &ifs)
{
    Triangle t;
    while (ifs.good())
    {
        std::string token;
        ifs >> token;
        if (token == "facet")
        {
            ifs >> token; // expecting "normal"
            ifs >> t.normal.x >> t.normal.y >> t.normal.z;
            ifs >> token; // expecting "outer"
            ifs >> token; // expecting "loop"
            for (int i = 0; i < 3; i++)
            {
                ifs >> token; // expecting "vertex"
                ifs >> t.vertices[i].x >> t.vertices[i].y >> t.vertices[i].z;
            }
            ifs >> token; // expecting "endloop"
            ifs >> token; // expecting "endfacet"
        }
    }
}

void read_stl_mesh_file(std::ifstream &ifs)
{
    size_t file_size = calc_file_size(ifs);
    if (file_size == 0)
    {
        std::cerr << "Empty file" << std::endl;
        return;
    }
    // Skip binary header
    ifs.seekg(BINARY_STL_HEADER_SIZE, std::ifstream::beg);

    uint32_t num_triangles = 0;
    ifs.read((char *)&num_triangles, sizeof(uint32_t));

    if (file_size == calc_expected_stl_mesh_file_binary_size(num_triangles))
    {
        read_stl_mesh_file_binary(ifs, num_triangles);
    }
    else
    {
        ifs.seekg(0, std::ifstream::beg);
        read_stl_mesh_file_ascii(ifs);
    }
}

void on_load_stl_dialog_ok(const std::string &file_path)
{
    std::ifstream ifs;
    ifs.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    try
    {
        ifs.open(file_path, std::ifstream::binary);
    }
    catch (std::ifstream::failure &)
    {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return;
    }
    std::vector<Vec3f> vertices;
    read_stl_mesh_file(ifs);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
}

void render()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(INITIAL_IMGUI_WINDOW_SIZE, ImGuiCond_Once);

    ImGui::Begin("GeoBox", nullptr, ImGuiWindowFlags_NoMove);

    if (ImGui::Button(LOAD_STL_BUTTON_DIALOG_TITLE))
    {
        ImGuiFileDialog::Instance()->OpenDialog(LOAD_STL_DIALOG_KEY, LOAD_STL_BUTTON_DIALOG_TITLE, ".stl", ".");
    }

    if (ImGuiFileDialog::Instance()->Display(LOAD_STL_DIALOG_KEY))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
            on_load_stl_dialog_ok(file_path);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void window_refresh_callback(GLFWwindow *window)
{
    render();
    glfwSwapBuffers(window);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void init_imgui(GLFWwindow *window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}

int main()
{
    // Initialize GLFW

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create Window

    GLFWwindow *window = glfwCreateWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (window == nullptr)
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

    // Initialize ImGui
    init_imgui(window);

    // Set OpenGL viewport

    glViewport(0, 0, INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);

    // Set window resize and refresh callbacks

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowRefreshCallback(window, window_refresh_callback);

    // Main loop

    while (!glfwWindowShouldClose(window))
    {
        // Poll events
        glfwPollEvents();

        // Process Input
        process_input(window);

        // Update state

        // Render to framebuffer
        render();

        // Swap/Present framebuffer to screen
        glfwSwapBuffers(window);

        // Delay to control FPS if needed
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}