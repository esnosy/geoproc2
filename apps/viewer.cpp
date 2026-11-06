#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include <array>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../libs/io/stl/stl_io.hpp"
#include "../libs/vec3.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

struct Vertex {
  Vec3 position;
  Vec3 normal;
};

struct Vertex_UBO {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 model_normal;
};

struct Fragment_UBO {
  glm::vec4 light_pos;
  glm::vec4 light_color;
  glm::vec4 object_color;
  glm::vec4 ambient;
};

SDL_Window *g_window = nullptr;
SDL_GPUDevice *g_device = nullptr;
SDL_GPUGraphicsPipeline *g_pipeline = nullptr;
Uint32 g_num_vertices = 0;
SDL_GPUBuffer *g_vertex_buffer = nullptr;
SDL_GPUShader *g_vertex_shader = nullptr;
SDL_GPUShader *g_fragment_shader = nullptr;
float g_x_rotation = 0;
float g_y_rotation = 0;
SDL_GPUTexture *g_depth_texture = nullptr;
Uint32 g_swapchain_width = 0;
Uint32 g_swapchain_height = 0;
bool g_imgui_init = false;
char *g_model_path = nullptr;
bool g_model_load_requested = false;
float g_camera_z = 3.0f;

static const SDL_DialogFileFilter import_mesh_file_dialog_filters[] = {
    {"STL", "stl"}, {"All files", "*"}};

SDL_GPUShader *load_shader(SDL_GPUDevice *device,
                           const std::string &shader_filename,
                           Uint32 num_uniform_buffers) {
  SDL_GPUShaderStage stage;
  if (shader_filename.contains(".vert")) {
    stage = SDL_GPU_SHADERSTAGE_VERTEX;
  } else if (shader_filename.contains(".frag")) {
    stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  } else {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to deduce shader stage from filename: %s",
                 shader_filename.c_str());
    return nullptr;
  }
  std::filesystem::path full_path = std::filesystem::path(SDL_GetBasePath());
  SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
  const char *entrypoint;
  SDL_GPUShaderFormat backend_formats = SDL_GetGPUShaderFormats(device);
  if (backend_formats & SDL_GPU_SHADERFORMAT_SPIRV) {
    full_path /= shader_filename + ".spv";
    format = SDL_GPU_SHADERFORMAT_SPIRV;
    entrypoint = "main";
  } else if (backend_formats & SDL_GPU_SHADERFORMAT_MSL) {
    full_path /= shader_filename + ".msl";
    format = SDL_GPU_SHADERFORMAT_MSL;
    entrypoint = "main0";
  } else {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to find supported shader format for backend %s",
                 SDL_GetGPUDeviceDriver(device));
    return nullptr;
  }
  size_t file_size;
  void *code = SDL_LoadFile(full_path.string().c_str(), &file_size);
  if (code == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to load shader file from disk: %s", SDL_GetError());
    return nullptr;
  }
  SDL_GPUShaderCreateInfo shader_info = SDL_GPUShaderCreateInfo{
      .code_size = file_size,
      .code = static_cast<Uint8 *>(code),
      .entrypoint = entrypoint,
      .format = format,
      .stage = stage,
      .num_uniform_buffers = num_uniform_buffers,
  };
  SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shader_info);
  if (shader == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to create shader from file %s: %s", full_path.c_str(),
                 SDL_GetError());
    SDL_free(code);
    return nullptr;
  }
  return shader;
}

bool create_pipeline() {
  g_vertex_shader = load_shader(g_device, "shader.vert", 1);
  if (g_vertex_shader == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create vertex shader!");
    return false;
  }

  g_fragment_shader = load_shader(g_device, "shader.frag", 1);
  if (g_fragment_shader == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create fragment shader!");
    return false;
  }
  std::array vertex_buffer_descriptions{
      SDL_GPUVertexBufferDescription{
          .slot = 0,
          .pitch = sizeof(Vertex),
          .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
          .instance_step_rate = 0,
      },
  };
  std::array vertex_attributes{
      SDL_GPUVertexAttribute{
          .location = 0,
          .buffer_slot = 0,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
          .offset = 0,
      },
      SDL_GPUVertexAttribute{
          .location = 1,
          .buffer_slot = 0,
          .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
          .offset = sizeof(Vec3),
      },
  };
  std::array color_target_descriptions{SDL_GPUColorTargetDescription{
      .format = SDL_GetGPUSwapchainTextureFormat(g_device, g_window)}};

  SDL_GPUDepthStencilState depth_state{};
  depth_state.enable_depth_test = true;
  depth_state.enable_depth_write = true;
  depth_state.compare_op = SDL_GPU_COMPAREOP_LESS;

  SDL_GPUGraphicsPipelineCreateInfo pipeline_create_info =
      SDL_GPUGraphicsPipelineCreateInfo{
          .vertex_shader = g_vertex_shader,
          .fragment_shader = g_fragment_shader,
          .vertex_input_state =
              SDL_GPUVertexInputState{
                  .vertex_buffer_descriptions =
                      vertex_buffer_descriptions.data(),
                  .num_vertex_buffers = vertex_buffer_descriptions.size(),
                  .vertex_attributes = vertex_attributes.data(),
                  .num_vertex_attributes = vertex_attributes.size(),
              },
          .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
          .rasterizer_state =
              SDL_GPURasterizerState{
                  .fill_mode = SDL_GPU_FILLMODE_FILL,
                  .cull_mode = SDL_GPU_CULLMODE_BACK,
                  .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
              },
          .depth_stencil_state = depth_state,
          .target_info =
              SDL_GPUGraphicsPipelineTargetInfo{
                  .color_target_descriptions = color_target_descriptions.data(),
                  .num_color_targets = color_target_descriptions.size(),
                  .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
                  .has_depth_stencil_target = true,
              },
  };
  g_pipeline = SDL_CreateGPUGraphicsPipeline(g_device, &pipeline_create_info);
  if (g_pipeline == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to create graphics pipeline: %s", SDL_GetError());
    return false;
  }
  SDL_ReleaseGPUShader(g_device, g_vertex_shader);
  SDL_ReleaseGPUShader(g_device, g_fragment_shader);
  return true;
}

bool create_vertex_buffer(std::span<Vertex> vertices) {
  g_num_vertices = vertices.size();
  Uint32 vertices_size = g_num_vertices * sizeof(Vertex);
  SDL_GPUBufferCreateInfo vertex_buffer_create_info = SDL_GPUBufferCreateInfo{
      .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
      .size = vertices_size,
  };
  g_vertex_buffer = SDL_CreateGPUBuffer(g_device, &vertex_buffer_create_info);
  if (g_vertex_buffer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create vertex buffer: %s",
                 SDL_GetError());
    return false;
  }
  SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info =
      SDL_GPUTransferBufferCreateInfo{
          .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
          .size = vertices_size,
  };
  SDL_GPUTransferBuffer *transfer_buffer =
      SDL_CreateGPUTransferBuffer(g_device, &transfer_buffer_create_info);
  if (transfer_buffer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create transfer buffer: %s",
                 SDL_GetError());
    return false;
  }
  Vertex *transfer_data = static_cast<Vertex *>(
      SDL_MapGPUTransferBuffer(g_device, transfer_buffer, false));
  if (transfer_data == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to map transfer buffer: %s",
                 SDL_GetError());
    SDL_ReleaseGPUTransferBuffer(g_device, transfer_buffer);
    return false;
  }
  SDL_memcpy(transfer_data, vertices.data(), vertices_size);
  SDL_UnmapGPUTransferBuffer(g_device, transfer_buffer);

  SDL_GPUCommandBuffer *upload_command_buffer =
      SDL_AcquireGPUCommandBuffer(g_device);
  if (upload_command_buffer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to acquire GPU command buffer: %s", SDL_GetError());
    return false;
  }
  SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(upload_command_buffer);
  SDL_GPUTransferBufferLocation buffer_location = SDL_GPUTransferBufferLocation{
      .transfer_buffer = transfer_buffer,
      .offset = 0,
  };
  SDL_GPUBufferRegion buffer_region = SDL_GPUBufferRegion{
      .buffer = g_vertex_buffer,
      .offset = 0,
      .size = vertices_size,
  };
  SDL_UploadToGPUBuffer(copy_pass, &buffer_location, &buffer_region, false);
  SDL_EndGPUCopyPass(copy_pass);
  if (!SDL_SubmitGPUCommandBuffer(upload_command_buffer)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to submit GPU command buffer: %s", SDL_GetError());
    return false;
  }
  SDL_ReleaseGPUTransferBuffer(g_device, transfer_buffer);
  return true;
}

static void SDLCALL import_mesh_file_dialog_callback(
    void *userdata, const char *const *filelist, int filter) {
  if (!filelist) {
    SDL_Log("An error occured: %s", SDL_GetError());
    return;
  } else if (!*filelist) {
    SDL_Log("The user did not select any file.");
    SDL_Log("Most likely, the dialog was canceled.");
    return;
  }

  while (*filelist) {
    SDL_Log("Full path to selected file: '%s'", *filelist);

    auto l = SDL_strlen(*filelist) + 1;
    if (g_model_path != nullptr) {
      SDL_free(g_model_path);
    }
    g_model_path = (char *)SDL_malloc(l);
    SDL_memcpy(g_model_path, *filelist, l);
    g_model_load_requested = true;

    filelist++;
  }

  if (filter < 0) {
    SDL_Log("The current platform does not support fetching "
            "the selected filter, or the user did not select"
            " any filter.");
  } else if (filter < SDL_arraysize(import_mesh_file_dialog_filters)) {
    SDL_Log("The filter selected by the user is '%s' (%s).",
            import_mesh_file_dialog_filters[filter].pattern,
            import_mesh_file_dialog_filters[filter].name);
  }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to init SDL: %s\n",
                 SDL_GetError());
    return SDL_APP_FAILURE;
  }

  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  SDL_WindowFlags window_flags =
      SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  g_window = SDL_CreateWindow("Viewer", (int)(800 * main_scale),
                              (int)(600 * main_scale), window_flags);
  if (g_window == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create window: %s",
                 SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED);

  SDL_GPUShaderFormat shader_format_flags =
      SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL;
  g_device = SDL_CreateGPUDevice(shader_format_flags, true, nullptr);
  if (g_device == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create GPU device: %s",
                 SDL_GetError());
    return SDL_APP_FAILURE;
  }
  if (!SDL_ClaimWindowForGPUDevice(g_device, g_window)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to claim window for GPU device: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_SetGPUSwapchainParameters(g_device, g_window,
                                SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                SDL_GPU_PRESENTMODE_VSYNC);

  if (!create_pipeline()) {
    return SDL_APP_FAILURE;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForSDLGPU(g_window);
  ImGui_ImplSDLGPU3_InitInfo init_info = {};
  init_info.Device = g_device;
  init_info.ColorTargetFormat =
      SDL_GetGPUSwapchainTextureFormat(g_device, g_window);
  init_info.MSAASamples =
      SDL_GPU_SAMPLECOUNT_1; // Only used in multi-viewports mode.
  init_info.SwapchainComposition =
      SDL_GPU_SWAPCHAINCOMPOSITION_SDR; // Only used in multi-viewports mode.
  init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
  ImGui_ImplSDLGPU3_Init(&init_info);

  g_imgui_init = true;

  SDL_ShowWindow(g_window);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplSDL3_ProcessEvent(event);
  switch (event->type) {
  case SDL_EVENT_QUIT:
    // Quit the application with a success state
    return SDL_APP_SUCCESS;
  case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
    if (event->window.windowID == SDL_GetWindowID(g_window)) {
      return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
  case SDL_EVENT_KEY_DOWN:
    if (io.WantCaptureKeyboard) {
      return SDL_APP_CONTINUE;
    }
    switch (event->key.key) {
    case SDLK_UP:
      g_x_rotation -= 5.0f;
      break;
    case SDLK_DOWN:
      g_x_rotation += 5.0f;
      break;
    case SDLK_LEFT:
      g_y_rotation -= 5.0f;
      break;
    case SDLK_RIGHT:
      g_y_rotation += 5.0f;
      break;
    case SDLK_W:
      g_camera_z -= 1.0f;
      break;
    case SDLK_S:
      g_camera_z += 1.0f;
      break;
    }
    return SDL_APP_CONTINUE;
  default:
    return SDL_APP_CONTINUE;
  }
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  if (SDL_GetWindowFlags(g_window) & SDL_WINDOW_MINIMIZED) {
    SDL_Delay(10);
    return SDL_APP_CONTINUE;
  }
  ImGui_ImplSDLGPU3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  {
    ImGui::Begin("Commands");
    if (ImGui::Button("Import mesh")) {
      SDL_ShowOpenFileDialog(import_mesh_file_dialog_callback, nullptr,
                             g_window, import_mesh_file_dialog_filters,
                             SDL_arraysize(import_mesh_file_dialog_filters),
                             nullptr, false);
    }
    ImGui::End();
  }

  ImGui::Render();

  if (g_model_load_requested) {
    g_model_load_requested = false;
    if (g_vertex_buffer != nullptr) {
      SDL_ReleaseGPUBuffer(g_device, g_vertex_buffer);
    }

    auto tris = read_stl(g_model_path);

    std::vector<Vertex> vertices;
    vertices.reserve(tris.size() * 3);
    for (const auto &t : tris) {
      auto ab = t.b - t.a;
      auto ac = t.c - t.a;
      auto normal = ab.cross(ac);
      normal.normalize();
      vertices.push_back({t.a, normal});
      vertices.push_back({t.b, normal});
      vertices.push_back({t.c, normal});
    }

    if (!create_vertex_buffer(vertices)) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create vertex buffer!");
      return SDL_APP_FAILURE;
    }
  }

  SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(g_device);
  if (command_buffer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to acquire GPU command buffer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  Uint32 swapchain_width, swapchain_height;
  SDL_GPUTexture *swapchain_texture;
  if (!SDL_WaitAndAcquireGPUSwapchainTexture(
          command_buffer, g_window, &swapchain_texture, &swapchain_width,
          &swapchain_height)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "Failed to acquire swapchain texture: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  if (swapchain_texture == nullptr) {
    SDL_CancelGPUCommandBuffer(command_buffer);
    return SDL_APP_CONTINUE;
  }
  ImDrawData *draw_data = ImGui::GetDrawData();
  const bool is_minimized =
      (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
  if (is_minimized) {
    SDL_CancelGPUCommandBuffer(command_buffer);
    return SDL_APP_CONTINUE;
  }
  ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

  if (g_swapchain_width != swapchain_width ||
      g_swapchain_height != swapchain_height) {
    if (g_depth_texture != nullptr) {
      SDL_ReleaseGPUTexture(g_device, g_depth_texture);
    }
    g_swapchain_width = swapchain_width;
    g_swapchain_height = swapchain_height;

    SDL_GPUTextureCreateInfo depth_info;
    depth_info.type = SDL_GPU_TEXTURETYPE_2D;
    depth_info.width = swapchain_width;
    depth_info.height = swapchain_height;
    depth_info.layer_count_or_depth = 1;
    depth_info.num_levels = 1;
    depth_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    depth_info.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
    depth_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;

    g_depth_texture = SDL_CreateGPUTexture(g_device, &depth_info);
    if (g_depth_texture == nullptr) {
      SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                   "Failed to create depth texture with new size (%d x %d): %s",
                   swapchain_width, swapchain_height, SDL_GetError());
      return SDL_APP_FAILURE;
    }
  }
  SDL_GPUColorTargetInfo color_target_info = {0};
  color_target_info.texture = swapchain_texture;
  color_target_info.clear_color = {0.4f, 0.6f, 0.9f, 1.0f};
  color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
  color_target_info.store_op = SDL_GPU_STOREOP_STORE;

  SDL_GPUDepthStencilTargetInfo depth_target_info{};
  depth_target_info.texture = g_depth_texture;
  depth_target_info.clear_depth = 1.0f;
  depth_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
  depth_target_info.store_op = SDL_GPU_STOREOP_STORE;
  depth_target_info.cycle = false;

  SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
      command_buffer, &color_target_info, 1, &depth_target_info);

  SDL_BindGPUGraphicsPipeline(render_pass, g_pipeline);
  if (g_vertex_buffer != nullptr) {
    std::array vertex_buffers{
        SDL_GPUBufferBinding{
            .buffer = g_vertex_buffer,
            .offset = 0,
        },
    };
    SDL_BindGPUVertexBuffers(render_pass, 0, vertex_buffers.data(),
                             vertex_buffers.size());

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), (float)swapchain_width / (float)swapchain_height,
        0.1f, 1000000.0f);
    glm::mat4 view =
        glm::lookAt(glm::vec3(0.0f, 0.0f, g_camera_z),
                    glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(g_x_rotation),
                        glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(g_y_rotation),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    Vertex_UBO vertex_ubo;
    vertex_ubo.model = model;
    vertex_ubo.view = view;
    vertex_ubo.projection = projection;
    vertex_ubo.model_normal = glm::transpose(glm::inverse(model));

    SDL_PushGPUVertexUniformData(command_buffer, 0, &vertex_ubo,
                                 sizeof(vertex_ubo));

    Fragment_UBO fragment_ubo;
    fragment_ubo.light_pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    fragment_ubo.light_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    fragment_ubo.object_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    fragment_ubo.ambient = glm::vec4(0.1f, 0.0f, 0.0f, 1.0f);

    SDL_PushGPUFragmentUniformData(command_buffer, 0, &fragment_ubo,
                                   sizeof(fragment_ubo));

    SDL_DrawGPUPrimitives(render_pass, g_num_vertices, 1, 0, 0);
  }

  SDL_EndGPURenderPass(render_pass);

  SDL_GPUColorTargetInfo imgui_color_target_info = {0};
  imgui_color_target_info.texture = swapchain_texture;
  imgui_color_target_info.load_op = SDL_GPU_LOADOP_LOAD;
  imgui_color_target_info.store_op = SDL_GPU_STOREOP_STORE;

  SDL_GPURenderPass *imgui_render_pass = SDL_BeginGPURenderPass(
      command_buffer, &imgui_color_target_info, 1, nullptr);
  ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer,
                                   imgui_render_pass);
  SDL_EndGPURenderPass(imgui_render_pass);
  SDL_SubmitGPUCommandBuffer(command_buffer);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  if (g_imgui_init) {
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();
  }

  if (g_device != nullptr) {
    if (g_depth_texture != nullptr) {
      SDL_ReleaseGPUTexture(g_device, g_depth_texture);
    }
    if (g_window != nullptr) {
      SDL_ReleaseWindowFromGPUDevice(g_device, g_window);
    }
    if (g_pipeline != nullptr) {
      SDL_ReleaseGPUGraphicsPipeline(g_device, g_pipeline);
    }
    if (g_vertex_buffer != nullptr) {
      SDL_ReleaseGPUBuffer(g_device, g_vertex_buffer);
    }
    SDL_DestroyGPUDevice(g_device);
  }
  if (g_window != nullptr) {
    SDL_DestroyWindow(g_window);
  }

  if (g_model_path != nullptr) {
    SDL_free(g_model_path);
  }
}