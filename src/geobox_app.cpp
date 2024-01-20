#include <cmath>
#include <cstdlib> // for std::exit
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

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

#ifdef ENABLE_SUPERLUMINAL_PERF_API
#include <Superluminal/PerformanceAPI.h>
#endif

constexpr int INIT_WINDOW_WIDTH = 800;
constexpr int INIT_WINDOW_HEIGHT = 600;
constexpr const char *WINDOW_TITLE = "GeoBox";

constexpr const char *IMGUI_MAIN_WINDOW_TITLE = WINDOW_TITLE;
constexpr ImVec2 INITIAL_IMGUI_MAIN_WINDOW_SIZE(550, 680);

constexpr const char *LOAD_STL_DIALOG_KEY = "Load_STL_Dialog_Key";
constexpr const char *LOAD_STL_BUTTON_DIALOG_TITLE = "Load .stl";

constexpr size_t BINARY_STL_HEADER_SIZE = 80;

constexpr ImVec2 INITIAL_IMGUI_FILE_DIALOG_WINDOW_OFFSET(100, 100);
constexpr ImVec2 INITIAL_IMGUI_FILE_DIALOG_WINDOW_SIZE(600, 500);

[[maybe_unused]] static size_t calc_file_size(std::ifstream &ifs) {
  auto original_pos = ifs.tellg();
  ifs.seekg(0, std::ifstream::end);
  auto end = ifs.tellg();
  ifs.seekg(0, std::ifstream::beg);
  size_t file_size = end - ifs.tellg();
  ifs.seekg(original_pos, std::ifstream::beg);
  return file_size;
}

static size_t calc_expected_binary_stl_mesh_file_size(uint32_t num_triangles) {
  return BINARY_STL_HEADER_SIZE + sizeof(uint32_t) + num_triangles * (sizeof(float[4][3]) + sizeof(uint16_t));
}

static std::vector<glm::vec3> read_stl_mesh_file_binary(std::ifstream &ifs, uint32_t num_triangles) {
  std::vector<glm::vec3> vertices;
  vertices.reserve(num_triangles * 3);
  for (uint32_t i = 0; i < num_triangles; i++) {
    ifs.seekg(sizeof(glm::vec3), std::ifstream::cur); // Skip normals
    for (int j = 0; j < 3; j++) {
      ifs.read((char *)&vertices.emplace_back(), sizeof(glm::vec3));
    }
    // Skip "attribute byte count"
    ifs.seekg(sizeof(uint16_t), std::ifstream::cur);
  }
  return vertices;
}

static std::vector<glm::vec3> read_stl_mesh_file_ascii(std::ifstream &ifs) {
  std::vector<glm::vec3> vertices;
  std::string token;
  while (ifs.good()) {
    ifs >> token;
    if (token == "facet") {
      ifs >> token;                   // expecting "normal"
      ifs >> token >> token >> token; // Skip normal
      ifs >> token;                   // expecting "outer"
      ifs >> token;                   // expecting "loop"
      for (int i = 0; i < 3; i++) {
        ifs >> token; // expecting "vertex"
        glm::vec3 &vertex = vertices.emplace_back();
        ifs >> vertex.x >> vertex.y >> vertex.z;
      }
      ifs >> token; // expecting "endloop"
      ifs >> token; // expecting "endfacet"
    }
  }
  return vertices;
}

static std::optional<std::vector<glm::vec3>> read_stl_mesh_file(const std::string &file_path) {
  std::ifstream ifs(file_path, std::ifstream::ate | std::ifstream::binary);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open file: " << file_path << std::endl;
    return {};
  }

  auto file_end = ifs.tellg(); // file is already at end if it was opened in "ate" mode
  ifs.seekg(0, std::ifstream::beg);
  auto file_beg = ifs.tellg();
  auto file_size = file_end - file_beg;

  if (file_size == 0) {
    std::cerr << "Empty file: " << file_path << std::endl;
    return {};
  }
  // Assume file is binary at first and skip binary header
  ifs.seekg(BINARY_STL_HEADER_SIZE, std::ifstream::beg);

  uint32_t num_triangles = 0;
  ifs.read((char *)&num_triangles, sizeof(uint32_t));

  if (file_size == calc_expected_binary_stl_mesh_file_size(num_triangles)) {
    return read_stl_mesh_file_binary(ifs, num_triangles);
  } else {
    ifs.seekg(0, std::ifstream::beg);
    return read_stl_mesh_file_ascii(ifs);
  }
}

static std::optional<std::string> read_file(const std::string &file_path) {
  std::ifstream ifs(file_path);
  if (!ifs.is_open()) {
    std::cerr << "Failed to open file: " << file_path << std::endl;
    return {};
  }
  return std::string(std::istreambuf_iterator(ifs), {});
}

glm::vec3 closest_point_on_aabb(glm::vec3 const &point, BVH::Node::AABB const &aabb) {
  return glm::clamp(point, aabb.min, aabb.max);
}

float point_aabb_distance_squared(glm::vec3 const &point, BVH::Node::AABB const &aabb) {
  return glm::distance2(point, closest_point_on_aabb(point, aabb));
}

struct Sphere {
  glm::vec3 center;
  float radius;
};

bool sphere_aabb_intersection(Sphere const &sphere, BVH::Node::AABB const &aabb) {
  return point_aabb_distance_squared(sphere.center, aabb) <= (sphere.radius * sphere.radius);
}

GeoBox_App::GeoBox_App() {
  init_glfw();
  init_glad();
  init_glfw_callbacks();
  // NOTE: it is important to initialize ImGUI after initializing GLFW callbacks, otherwise setting GLFW callbacks will override ImGUI callbacks,
  // ImGUI smartly chains its callbacks to any existing callbacks, so we initialize our callbacks first, and let ImGUI chain its callbacks
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

  // Swap interval of 1: reduces tearing with minimal input latency
  glfwSwapInterval(1);
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

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(m_window, true); // Second param install_callback=true will install GLFW
                                                // callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();
}

bool GeoBox_App::init_shaders() {
  std::optional<std::string> default_vertex_shader_source = read_file("resources/shaders/default.vert");
  if (!default_vertex_shader_source.has_value()) {
    return false;
  }
  std::optional<std::string> default_fragment_shader_source = read_file("resources/shaders/default.frag");
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

void window_refresh_callback(GLFWwindow *window) {
  auto app = static_cast<GeoBox_App *>(glfwGetWindowUserPointer(window));
  app->render();
  glfwSwapBuffers(window);
  glFinish();
}

void framebuffer_size_callback(const GLFWwindow * /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

void GeoBox_App::init_glfw_callbacks() {
  // Using methods as callbacks: https://stackoverflow.com/a/28660673/8094047
  glfwSetFramebufferSizeCallback(m_window, (GLFWframebuffersizefun)framebuffer_size_callback);
  glfwSetWindowRefreshCallback(m_window, (GLFWwindowrefreshfun)window_refresh_callback);
}

void GeoBox_App::process_input() {
  if (const ImGuiIO &imgui_io = ImGui::GetIO(); imgui_io.WantCaptureMouse || imgui_io.WantCaptureKeyboard)
    return;
  const float camera_speed = 2.5f * m_delta_time * m_camera_orbit_radius;
  if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
    m_camera_orbit_radius = glm::max(m_camera_orbit_radius - camera_speed, 0.0f);
  }
  if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
    m_camera_orbit_radius += camera_speed;
  }
  if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
    // TODO: do something
  }
  if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
    // TODO: do something
  }
  if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    double x_pos;
    double y_pos;
    glfwGetCursorPos(m_window, &x_pos, &y_pos);
    if (m_last_mouse_pos.has_value()) {
      float x_offset = static_cast<float>(x_pos) - m_last_mouse_pos->x;
      float y_offset =
          m_last_mouse_pos->y - static_cast<float>(y_pos); // reversed since y-coordinates range from bottom to top
      float mouse_sensitivity = glm::radians(20.0f) * m_delta_time;
      x_offset *= mouse_sensitivity;
      y_offset *= mouse_sensitivity;
      m_camera_inclination -= y_offset;
      m_camera_azimuth += x_offset;
    }
    m_last_mouse_pos = glm::vec2(x_pos, y_pos);
  } else {
    m_last_mouse_pos.reset();
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
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

  glm::mat4 model(1.0f);

  // Spherical coordinates unit vectors: https://mathworld.wolfram.com/SphericalCoordinates.html
  float sin_camera_inclination = glm::sin(m_camera_inclination);
  float cos_camera_inclination = glm::cos(m_camera_inclination);
  float sin_camera_azimuth = glm::sin(m_camera_azimuth);
  float cos_camera_azimuth = glm::cos(m_camera_azimuth);
  glm::vec3 orbit_sphere_normal{
      sin_camera_inclination * cos_camera_azimuth,
      sin_camera_inclination * sin_camera_azimuth,
      cos_camera_inclination,
  };
  glm::vec3 orbit_sphere_tangent{
      -1 * sin_camera_azimuth,
      cos_camera_azimuth,
      0,
  };
  glm::vec3 orbit_sphere_bi_tangent{
      cos_camera_inclination * cos_camera_azimuth,
      cos_camera_inclination * sin_camera_azimuth,
      -sin_camera_inclination,
  };
  glm::vec3 camera_pos = orbit_sphere_normal * m_camera_orbit_radius;
  glm::mat3 camera_basis{
      orbit_sphere_tangent,
      orbit_sphere_bi_tangent,
      orbit_sphere_normal,
  };
  glm::mat4 camera(1.0f);
  camera = glm::translate(camera, camera_pos);
  camera = camera * glm::mat4(camera_basis);

  glm::mat4 view = glm::inverse(camera);
  glm::mat4 projection =
      glm::perspective(glm::radians(m_camera_fov_degrees), (float)width / (float)height, 0.01f, 1000.0f);

  int model_matrix_uniform_location = glGetUniformLocation(m_default_shader_program, "model");
  glUniformMatrix4fv(model_matrix_uniform_location, 1, GL_FALSE, glm::value_ptr(model));
  int view_matrix_uniform_location = glGetUniformLocation(m_default_shader_program, "view");
  glUniformMatrix4fv(view_matrix_uniform_location, 1, GL_FALSE, glm::value_ptr(view));
  int projection_matrix_uniform_location = glGetUniformLocation(m_default_shader_program, "projection");
  glUniformMatrix4fv(projection_matrix_uniform_location, 1, GL_FALSE, glm::value_ptr(projection));

  if (m_is_object_loaded) {
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT, nullptr);
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  const ImGuiViewport *main_viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Once);
  ImGui::SetNextWindowSize(INITIAL_IMGUI_MAIN_WINDOW_SIZE, ImGuiCond_Once);

  ImGui::Begin(IMGUI_MAIN_WINDOW_TITLE, nullptr, ImGuiWindowFlags_None);

  if (ImGui::Button(LOAD_STL_BUTTON_DIALOG_TITLE)) {
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + INITIAL_IMGUI_FILE_DIALOG_WINDOW_OFFSET.x,
                                   main_viewport->WorkPos.y + INITIAL_IMGUI_FILE_DIALOG_WINDOW_OFFSET.y),
                            ImGuiCond_Once);
    ImGui::SetNextWindowSize(INITIAL_IMGUI_FILE_DIALOG_WINDOW_SIZE, ImGuiCond_Once);

    ImGuiFileDialog::Instance()->OpenDialog(LOAD_STL_DIALOG_KEY, LOAD_STL_BUTTON_DIALOG_TITLE, ".stl", ".");
  }

  if (ImGuiFileDialog::Instance()->Display(LOAD_STL_DIALOG_KEY)) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
      on_load_stl_dialog_ok(file_path);
    }
    ImGuiFileDialog::Instance()->Close();
  }

  ImGui::End();

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
  BVH bvh(vertices.value());
  if (bvh.did_build_fail()) {
    std::cerr << "Failed to build BVH" << std::endl;
    return;
  }
  assert(bvh.count_primitives() == vertices->size());
  std::cout << "Num nodes = " << bvh.count_nodes() << std::endl;
  std::cout << "Max node size = " << bvh.calc_max_leaf_size() << std::endl;

  std::vector<unsigned int> indices(vertices->size());
  std::vector<bool> is_remapped_vec(vertices->size(), false);
  std::vector<glm::vec3> unique_vertices;
  unique_vertices.reserve(vertices->size());

  unsigned int final_unique_vertex_index = 0;
  for (unsigned int original_unique_vertex_index = 0; original_unique_vertex_index < vertices->size();
       original_unique_vertex_index++) {
    // Skip already remapped vertex
    if (is_remapped_vec[original_unique_vertex_index])
      continue;
    glm::vec3 const &unique_vertex = vertices.value()[original_unique_vertex_index];
    constexpr float range = 0.0001f;
    Sphere sphere{.center = unique_vertex, .radius = range};

    auto duplicate_vertex_callback = [final_unique_vertex_index, &is_remapped_vec,
                                      &indices](unsigned int duplicate_vertex_index) {
      if (!is_remapped_vec[duplicate_vertex_index]) {
        indices[duplicate_vertex_index] = final_unique_vertex_index;
        is_remapped_vec[duplicate_vertex_index] = true;
      }
    };
    auto aabb_filter = [&const_sphere = std::as_const(sphere)](BVH::Node::AABB const &aabb) {
      return sphere_aabb_intersection(const_sphere, aabb);
    };
    auto primitive_filter = [&const_vertices = std::as_const(vertices.value()),
                             &unique_vertex](unsigned int potential_duplicate_vertex_index) {
      return glm::distance2(unique_vertex, const_vertices[potential_duplicate_vertex_index]) <= (range * range);
    };
    bvh.foreach_primitive(duplicate_vertex_callback, aabb_filter, primitive_filter);

    final_unique_vertex_index++;
    unique_vertices.push_back(unique_vertex);
  }
  unique_vertices.shrink_to_fit();

  std::cout << "Num unique vertices = " << unique_vertices.size() << std::endl;

  // Pre-calculate number of triangles per vertex (can be used later for weighting normals)
  std::vector<float> num_triangles_per_vertex(unique_vertices.size(), 0.0f);
  for (unsigned int vi : indices) {
    if (num_triangles_per_vertex[vi] == std::numeric_limits<float>::max()) {
      std::cerr
          << "Number of triangles per vertex exceeds maximum value of a float, aborting vertex normals calculation"
          << std::endl
          << "Failed to load mesh" << std::endl;
      return;
    }
    num_triangles_per_vertex[vi] += 1;
  }

  std::vector<glm::vec3> vertex_normals(unique_vertices.size(), glm::vec3(0.0f));
  for (unsigned int i = 0; i < indices.size(); i += 3) {
    const glm::vec3 &a = unique_vertices[indices[i + 0]];
    const glm::vec3 &b = unique_vertices[indices[i + 1]];
    const glm::vec3 &c = unique_vertices[indices[i + 2]];
    glm::vec3 triangle_normal = glm::normalize(glm::cross(b - a, c - a));
    for (int j = 0; j < 3; j++) {
      unsigned int vi = indices[i + j];
      // Avoid overflow by dividing values while accumulating them
      vertex_normals[vi] += triangle_normal / num_triangles_per_vertex[vi];
    }
  }
  // Ensure normalized normals, since weighted sum of normals can not be guaranteed to be normalized
  // (some normals can be zero, weights might not sum up to 1.0f, etc...)
  for (glm::vec3 &vertex_normal : vertex_normals) {
    vertex_normal = glm::normalize(vertex_normal);
  }

  if (unique_vertices.size() > (std::numeric_limits<unsigned int>::max() / sizeof(glm::vec3))) {
    std::cerr << "Aborting GPU mesh creation, too many vertices, TODO: support larger meshes" << std::endl;
    return;
  }
  auto unique_vertices_buffer_size = static_cast<unsigned int>(unique_vertices.size() * sizeof(glm::vec3));

  if (indices.size() > (std::numeric_limits<int>::max() / sizeof(unsigned int))) {
    std::cerr << "Aborting GPU mesh creation, too many indices, TODO: support larger meshes" << std::endl;
    return;
  }

  // Set CPU mesh
  m_vertices = unique_vertices;
  m_indices = indices;
  m_vertex_normals = vertex_normals;

  // Free old GPU data if needed
  if (m_is_object_loaded) {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_vertex_positions_buffer_object);
    glDeleteBuffers(1, &m_vertex_normals_buffer_object);
    glDeleteBuffers(1, &m_EBO);
  }

  // Create new GPU mesh data

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  unsigned int vertex_positions_buffer_object;
  glGenBuffers(1, &vertex_positions_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_positions_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, unique_vertices_buffer_size, unique_vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(0);

  m_num_indices = static_cast<int>(indices.size());
  int indices_buffer_size = m_num_indices * static_cast<int>(sizeof(unsigned int));
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_buffer_size, indices.data(), GL_STATIC_DRAW);

  // Vertex normals buffer has same size as vertex positions buffer if calculated properly
  unsigned int vertex_normals_buffer_size = unique_vertices_buffer_size;
  unsigned int vertex_normals_buffer_object;
  glGenBuffers(1, &vertex_normals_buffer_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_normals_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, vertex_normals_buffer_size, m_vertex_normals.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(1);

  m_VAO = VAO;
  m_vertex_positions_buffer_object = vertex_positions_buffer_object;
  m_vertex_normals_buffer_object = vertex_normals_buffer_object;
  m_EBO = EBO;
  m_is_object_loaded = true;

#ifdef ENABLE_SUPERLUMINAL_PERF_API
  PerformanceAPI_EndEvent();
#endif
}
