#include <cmath>
#include <cstdlib> // for std::exit
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ImGuiFileDialog.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#include "bvh.hpp"
#include "geobox_app.hpp"
#include "read_stl.hpp"

#ifdef ENABLE_SUPERLUMINAL_PERF_API
#include <Superluminal/PerformanceAPI.h>
#endif

constexpr int INIT_WINDOW_WIDTH = 800;
constexpr int INIT_WINDOW_HEIGHT = 600;
constexpr const char *WINDOW_TITLE = "GeoBox";

constexpr const char *LOAD_STL_DIALOG_KEY = "Load_STL_Dialog_Key";
constexpr const char *LOAD_STL_BUTTON_AND_DIALOG_TITLE = "Load .stl";

constexpr ImVec2 INITIAL_IMGUI_FILE_DIALOG_WINDOW_OFFSET(100, 100);
constexpr ImVec2 INITIAL_IMGUI_FILE_DIALOG_WINDOW_SIZE(600, 500);

constexpr float CAMERA_ORBIT_ZOOM_SPEED_MULTIPLIER = 2.5f;
constexpr float CAMERA_PAN_SPEED_MULTIPLIER = 0.05f;
constexpr float CAMERA_ORBIT_ROTATE_SPEED_RADIANS = glm::radians(20.0f);
// Minimum value for orbit radius when calculating various camera movement speeds based on orbit radius
constexpr float MIN_ORBIT_RADIUS_AS_SPEED_MULTIPLIER = 0.1f;

constexpr int NUM_ANTIALIASING_SAMPLES = 8;

static std::optional<std::string> read_file_as_string(const std::string &file_path) {
  std::ifstream ifs(file_path, std::ifstream::binary);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open file: " << file_path << std::endl;
    return {};
  }
  return std::string(std::istreambuf_iterator(ifs), {});
}

GeoBox_App::GeoBox_App() {
  init_glfw();
  init_glad();
  init_glfw_callbacks();
  // NOTE: it is important to initialize ImGUI after initializing GLFW callbacks, otherwise setting GLFW callbacks will
  // override ImGUI callbacks, ImGUI smartly chains its callbacks to any existing callbacks, so we initialize our
  // callbacks first, and let ImGUI chain its callbacks
  init_imgui();
  if (!init_shaders()) {
    std::cerr << "Failed to initialize shaders" << std::endl;
    shutdown();
    std::exit(-1);
  }
  glEnable(GL_DEPTH_TEST);

  int width;
  int height;
  glfwGetFramebufferSize(m_window, &width, &height);
  glViewport(0, 0, width, height);

  // Enable anti-aliasing
  glEnable(GL_MULTISAMPLE);
}

void GeoBox_App::main_loop() {
  while (!glfwWindowShouldClose(m_window)) {
    // Update delta time
    auto current_frame_time = static_cast<float>(glfwGetTime());
    m_delta_time = current_frame_time - m_last_frame_time;
    m_last_frame_time = current_frame_time;

    // Poll events
    glfwPollEvents();

    // Process Input
    process_input();

    // Update state

    // Render to framebuffer
    render();

    // Swap/Present framebuffer to screen
    glfwSwapBuffers(m_window);

    // Delay to control FPS if needed
  }
  shutdown();
}

void GeoBox_App::init_glfw() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  glfwWindowHint(GLFW_SAMPLES, NUM_ANTIALIASING_SAMPLES);
  m_window = glfwCreateWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
  if (m_window == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    std::exit(-1);
  }
  glfwMakeContextCurrent(m_window);

  // Store a pointer to the object in the GLFW "user" pointer,
  // so we can access methods in GLFW callbacks, see
  // https://stackoverflow.com/a/28660673/8094047
  glfwSetWindowUserPointer(m_window, this);

  // Raw mouse motion, from GLFW documentation:
  // "Raw mouse motion is closer to the actual motion of the mouse across a surface. It is not affected by the scaling
  // and acceleration applied to the motion of the desktop cursor. That processing is suitable for a cursor while raw
  // motion is better for controlling for example a 3D camera. Because of this, raw mouse motion is only provided when
  // the cursor is disabled."
  if (glfwRawMouseMotionSupported()) {
    glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }
}

void GeoBox_App::init_glad() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    std::exit(-1);
  }
}

void GeoBox_App::init_imgui() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  // Disabling ini file: https://github.com/ocornut/imgui/issues/5169
  io.IniFilename = nullptr;
  io.LogFilename = nullptr;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable docking (only supported in ImGUI docking branch)

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(m_window, true); // Second param install_callback=true will install GLFW
                                                // callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();
}

bool GeoBox_App::init_shaders() {
  std::optional<std::string> default_vertex_shader_source = read_file_as_string("resources/shaders/phong.vert");
  if (!default_vertex_shader_source.has_value()) {
    return false;
  }
  std::optional<std::string> default_fragment_shader_source = read_file_as_string("resources/shaders/phong.frag");
  if (!default_fragment_shader_source.has_value()) {
    return false;
  }

  std::vector<char *> vertex_shader_sources = {default_vertex_shader_source->data()};
  std::vector<char *> fragment_shader_sources = {default_fragment_shader_source->data()};
  int success;

  int max_info_log_length = 512;
  int actual_info_log_length = 0;
  std::vector<char> info_log(max_info_log_length);

  unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, (int)vertex_shader_sources.size(), vertex_shader_sources.data(), nullptr);
  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, max_info_log_length, &actual_info_log_length, info_log.data());
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << std::string(info_log.begin(), info_log.begin() + actual_info_log_length) << std::endl;
    return false;
  }

  unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, (int)fragment_shader_sources.size(), fragment_shader_sources.data(), nullptr);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, max_info_log_length, &actual_info_log_length, info_log.data());
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << std::string(info_log.begin(), info_log.begin() + actual_info_log_length) << std::endl;
    return false;
  }

  unsigned int shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, max_info_log_length, &actual_info_log_length, info_log.data());
    std::cerr << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
              << std::string(info_log.begin(), info_log.begin() + actual_info_log_length) << std::endl;
    return false;
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  m_default_shader_program = shader_program;

  return true;
}

void framebuffer_size_callback(const GLFWwindow * /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

void GeoBox_App::init_glfw_callbacks() {
  // Using methods as callbacks: https://stackoverflow.com/a/28660673/8094047
  glfwSetFramebufferSizeCallback(m_window, (GLFWframebuffersizefun)framebuffer_size_callback);
}

void GeoBox_App::process_input() {
  if (const ImGuiIO &imgui_io = ImGui::GetIO(); imgui_io.WantCaptureMouse || imgui_io.WantCaptureKeyboard)
    return;
  float orbit_radius_as_speed_multiplier = glm::max(m_camera.get_orbit_radius(), MIN_ORBIT_RADIUS_AS_SPEED_MULTIPLIER);
  float camera_orbit_zoom_speed = CAMERA_ORBIT_ZOOM_SPEED_MULTIPLIER * orbit_radius_as_speed_multiplier * m_delta_time;
  float camera_orbit_zoom_offset = 0.0f;
  glm::vec3 camera_orbit_origin_offset(0.0f);
  float camera_inclination_offset = 0.0f;
  float camera_azimuth_offset = 0.0f;

  if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
    camera_orbit_zoom_offset = -1.0f * camera_orbit_zoom_speed;
  }
  if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
    camera_orbit_zoom_offset = camera_orbit_zoom_speed;
  }
  if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    double x_pos;
    double y_pos;
    glfwGetCursorPos(m_window, &x_pos, &y_pos);
    if (m_last_mouse_pos.has_value()) {
      float x_offset = static_cast<float>(x_pos) - m_last_mouse_pos->x;
      // reversed since y-coordinates range from bottom to top
      float y_offset = m_last_mouse_pos->y - static_cast<float>(y_pos);

      if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        // We negate the expression because we are moving the camera, and we want the scene to follow the mouse
        // movement, so we negate the camera movement to achieve that
        camera_orbit_origin_offset = -1.0f * (m_camera.get_right() * x_offset + m_camera.get_up() * y_offset) *
                                     CAMERA_PAN_SPEED_MULTIPLIER * m_delta_time * orbit_radius_as_speed_multiplier;
      } else {
        float orbit_speed = CAMERA_ORBIT_ROTATE_SPEED_RADIANS * m_delta_time;
        // if y_offset increases from bottom to top we increase inclination in the same direction,
        // increasing inclination angle lowers camera position, which is what we want,
        // we want the scene to orbit up when mouse is dragged up,
        // so we orbit the camera down to achieve the effect of orbiting the scene up
        camera_inclination_offset = y_offset * orbit_speed;
        // if x_offset increases from left to right, we want to decrease the camera's azimuth (also called polar angle),
        // this will cause the camera to rotate left around the orbit origin,
        // so that when the mouse moves right, the scene also appears rotating right,
        // which is what we want
        camera_azimuth_offset = -1.0f * x_offset * orbit_speed;
      }
    }
    m_last_mouse_pos = glm::vec2(x_pos, y_pos);
  } else {
    m_last_mouse_pos.reset();
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
  m_camera.update(camera_inclination_offset, camera_azimuth_offset, camera_orbit_zoom_offset,
                  camera_orbit_origin_offset);
}

void GeoBox_App::render() {
  int width;
  int height;
  glfwGetFramebufferSize(m_window, &width, &height);
  if (width == 0 || height == 0) {
    return;
  }

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(m_default_shader_program);

  int object_color_uniform_location = glGetUniformLocation(m_default_shader_program, "object_color");
  glUniform3f(object_color_uniform_location, 1.0f, 1.0f, 1.0f);

  int light_color_uniform_location = glGetUniformLocation(m_default_shader_program, "light_color");
  glUniform3f(light_color_uniform_location, 1.0f, 1.0f, 1.0f);

  glm::mat4 view = m_camera.get_view_matrix();
  glm::mat4 projection =
      glm::perspective(glm::radians(m_perspective_fov_degrees), (float)width / (float)height, 0.01f, 1000.0f);

  int view_matrix_uniform_location = glGetUniformLocation(m_default_shader_program, "view");
  glUniformMatrix4fv(view_matrix_uniform_location, 1, GL_FALSE, glm::value_ptr(view));
  int projection_matrix_uniform_location = glGetUniformLocation(m_default_shader_program, "projection");
  glUniformMatrix4fv(projection_matrix_uniform_location, 1, GL_FALSE, glm::value_ptr(projection));

  int camera_pos_uniform_location = glGetUniformLocation(m_default_shader_program, "camera_position");
  glUniform3fv(camera_pos_uniform_location, 1, glm::value_ptr(m_camera.get_camera_pos()));

  int model_matrix_uniform_location = glGetUniformLocation(m_default_shader_program, "model");
  for (const std::shared_ptr<Object> &object : m_objects) {
    glUniformMatrix4fv(model_matrix_uniform_location, 1, GL_FALSE, glm::value_ptr(object->get_model()));
    object->draw();
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  // Thanks!: https://github.com/ocornut/imgui/issues/6307
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem(LOAD_STL_BUTTON_AND_DIALOG_TITLE)) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog(LOAD_STL_DIALOG_KEY, LOAD_STL_BUTTON_AND_DIALOG_TITLE, ".stl", config);
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleVar();

  const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + INITIAL_IMGUI_FILE_DIALOG_WINDOW_OFFSET.x,
                                 main_viewport->WorkPos.y + INITIAL_IMGUI_FILE_DIALOG_WINDOW_OFFSET.y),
                          ImGuiCond_Once);
  ImGui::SetNextWindowSize(INITIAL_IMGUI_FILE_DIALOG_WINDOW_SIZE, ImGuiCond_Once);
  if (ImGuiFileDialog::Instance()->Display(LOAD_STL_DIALOG_KEY)) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
      on_load_stl_dialog_ok(file_path);
    }
    ImGuiFileDialog::Instance()->Close();
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GeoBox_App::shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
}

void GeoBox_App::on_load_stl_dialog_ok(const std::string &file_path) {
#ifdef ENABLE_SUPERLUMINAL_PERF_API
  PerformanceAPI_BeginEvent(__FUNCTION__, nullptr, PERFORMANCEAPI_DEFAULT_COLOR);
  PERFORMANCEAPI_INSTRUMENT_FUNCTION();
#endif
  std::optional<std::vector<glm::vec3>> vertices = read_stl_mesh_file(file_path);
  if (!vertices.has_value()) {
    std::cerr << "Failed to import .stl mesh file: " << file_path << std::endl;
    return;
  }
  if (vertices->empty()) {
    std::cerr << "Empty mesh: " << file_path << std::endl;
    return;
  }

  std::shared_ptr<Object> object = Object::from_triangles(vertices.value(), glm::mat4(1.0f));
  if (!object) {
    std::cerr << "Failed to create object" << std::endl;
    return;
  }
  m_objects.push_back(object);

#ifdef ENABLE_SUPERLUMINAL_PERF_API
  PerformanceAPI_EndEvent();
#endif
}
