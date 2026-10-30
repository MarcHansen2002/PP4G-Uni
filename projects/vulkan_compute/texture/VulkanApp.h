#pragma once

#include "../maths.h"            // for standard types, glm::ortho
#include "../utility.h"          // for DBG_ASSERT
#include "../vulkan_context.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_resources.h"
#include "SmokeSimulation/SmokeData.h"

#include <array>                  // for std::array
#include <cmath>                  // for std::ceilf
#include <span>                   // for std::span

#define VK_USE_PLATFORM_WIN32_KHR // tell vulkan we are on Windows platform
#include <vulkan/vulkan.h>        // for everything vulkan
#define WIN32_LEAN_AND_MEAN       // exclude rarely-used content from the Windows headers
#define NOMINMAX                  // prevent Windows macros defining their own min and max macros
#include <Windows.h>              // for WinMain

struct vertex_2d_uv
{
	vec2 pos; // 2d pos
	vec2 uv;  // texture coordinates
};
struct camera_buffer
{
	mat4 vp_matrix;
};
struct model_buffer
{
	mat4 model_matrix;
};
struct smoke_render_buffer
{
	float density[CellCount];
};

constexpr char const* WINDOW_TITLE = "vulkan_compute_texture";
constexpr char const* COMPILED_COMPUTE_SHADER_PATH = "data/shaders/glsl/vulkan_compute_texture/vulkan_compute_texture.comp.spv";
constexpr char const* COMPILED_GRAPHICS_SHADER_PATH_VERT = "data/shaders/glsl/vulkan_compute_texture/sprite.vert.spv";
constexpr char const* COMPILED_GRAPHICS_SHADER_PATH_FRAG = "data/shaders/glsl/vulkan_compute_texture/sprite.frag.spv";
constexpr char const* TEXTURE_PATH = "data/textures/test.png";

constexpr u32 NUM_SETS_COMPUTE = 1u;

constexpr u32 NUM_RESOURCES_COMPUTE_SET_0 = 2u;
constexpr u32 BINDING_ID_SET_0_TEXTURE_INPUT = 0u;
constexpr u32 BINDING_ID_SET_0_TEXTURE_OUTPUT = 1u;

constexpr u32 NUM_SETS_GRAPHICS = 2u;

constexpr u32 NUM_RESOURCES_GRAPHICS_SET_0 = 2u;
constexpr u32 BINDING_ID_SET_0_UBO_CAMERA = 0u;
constexpr u32 BINDING_ID_SET_0_UBO_MODEL = 1u;

constexpr u32 NUM_RESOURCES_GRAPHICS_SET_1 = 1u;
constexpr u32 BINDING_ID_SET_1_TEXTURE = 0u;

class VulkanApp
{
public:
	VulkanApp()
	{
		// Randomise densities for testing purposes
		// TODO: Don't
		for (int i = 0; i < CellCount; i++)
		{
			SmokeStateA.Density[i] = ((double)rand() / (RAND_MAX)) + 1;
		}
	}

	int Init();

	int CreateContext();

	int CreateComputePipeline();
	int CreateComputeDescriptorSets();
	int CreateComputeResources();
	int BindComputeResources();
	int CreateComputeCommandBuffers();
	int CreateComputeSyncObjects();
	int ComputeDispatch();

	int CreateGraphicsPipeline();
	int CreateGraphicsDescriptorSets();
	int CreateGraphicsResources();
	int BindGraphicsAndComputeResources();
	int CreateGraphicsCommandBuffers();
	int CreateGraphicsSyncObjects();

	int CreatePlane();

	int Render();
	int Submit();

	void Terminate();
private:
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkExtent2D extent = {};
	VkRenderPass render_pass = VK_NULL_HANDLE;

	VkQueue queue_compute = VK_NULL_HANDLE, queue_graphics = VK_NULL_HANDLE;

	std::array <VkDescriptorSetLayout, NUM_SETS_COMPUTE> descriptor_set_layouts_compute = { VK_NULL_HANDLE };
	VkPipelineLayout pipeline_layout_compute = VK_NULL_HANDLE;
	VkPipeline pipeline_compute = VK_NULL_HANDLE;

	VkDescriptorPool descriptor_pool_compute = VK_NULL_HANDLE;
	vulkan_descriptor_set desc_set_0_compute; //for compute, texture_compute_input + texture_compute_output

	vulkan_texture texture_compute_input;
	vulkan_texture texture_compute_output;

	VkCommandPool command_pool_compute = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer_compute = VK_NULL_HANDLE;

	VkFence fence_compute = VK_NULL_HANDLE;

	// Graphics
	std::array <VkDescriptorSetLayout, NUM_SETS_GRAPHICS> descriptor_set_layouts_graphics = { VK_NULL_HANDLE, VK_NULL_HANDLE };
	VkPipelineLayout pipeline_layout_graphics = VK_NULL_HANDLE;
	VkPipeline pipeline_graphics = VK_NULL_HANDLE;

	VkDescriptorPool descriptor_pool_graphics = VK_NULL_HANDLE;
	vulkan_descriptor_set desc_set_1_graphics_input;  // for rendering input image , texture_compute_input
	vulkan_descriptor_set desc_set_0_graphics_output; // for rendering output image, buffer_graphics_camera + buffer_graphics_model_output
	vulkan_descriptor_set desc_set_1_graphics_output; // for rendering output image, texture_compute_output

	vulkan_buffer buffer_graphics_camera;
	vulkan_buffer buffer_graphics_model_output; // for when rendering the output texture
	vulkan_buffer buffer_density_a;				// For storing density to put into the previous image
	vulkan_buffer buffer_density_b;				// For storing density to put into the rendered image

	VkCommandPool command_pool_graphics = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer_graphics = VK_NULL_HANDLE;

	vulkan_mesh mesh_sprite;

	VkSemaphore swapchain_image_available_semaphore = VK_NULL_HANDLE;
	VkFence fence_submit_graphics = VK_NULL_HANDLE;

	// Smoke Simulation
	SmokeState SmokeStateA;
	SmokeState SmokeStateB;
};