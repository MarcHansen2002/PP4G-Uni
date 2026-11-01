#pragma once

#include "../maths.h"            // for standard types, glm::ortho
#include "../utility.h"          // for DBG_ASSERT
#include "../vulkan_context.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_resources.h"
#include "SmokeSimulation/SmokeData.h"

#include "imgui.h"

#include <array>                  // for std::array
#include <cmath>                  // for std::ceilf
#include <span>                   // for std::span

#define VK_USE_PLATFORM_WIN32_KHR // tell vulkan we are on Windows platform
#include <vulkan/vulkan.h>        // for everything vulkan
#define WIN32_LEAN_AND_MEAN       // exclude rarely-used content from the Windows headers
#define NOMINMAX                  // prevent Windows macros defining their own min and max macros
#include <Windows.h>              // for WinMain

class VulkanApp;

constexpr u32 NUM_SETS_GRAPHICS = 2u;
constexpr char const* COMPILED_GRAPHICS_SHADER_PATH_VERT = "data/shaders/glsl/vulkan_compute_texture/sprite.vert.spv";
constexpr char const* COMPILED_GRAPHICS_SHADER_PATH_FRAG = "data/shaders/glsl/vulkan_compute_texture/sprite.frag.spv";

class VulkanApp_Graphics
{
public:
	VulkanApp_Graphics() {}

	VkCommandBuffer* GetCommandBuffer() { return &command_buffer_graphics; }
	VkQueue* GetQueue() { return &queue_graphics; }
	VkSemaphore* GetSemaphore() { return &swapchain_image_available_semaphore; }
	VkFence* GetFence() { return &fence_submit_graphics; }

	VulkanApp* VkApp = VK_NULL_HANDLE;

	

	vulkan_buffer buffer_graphics_camera;
	vulkan_buffer buffer_graphics_model_output; // for when rendering the output texture

	vulkan_texture render_texture;

	VkSemaphore swapchain_image_available_semaphore = VK_NULL_HANDLE;

	VkQueue queue_graphics = VK_NULL_HANDLE;

	std::array <VkDescriptorSetLayout, NUM_SETS_GRAPHICS> descriptor_set_layouts_graphics = { VK_NULL_HANDLE, VK_NULL_HANDLE };

	VkPipelineLayout pipeline_layout_graphics = VK_NULL_HANDLE;
	VkPipeline pipeline_graphics = VK_NULL_HANDLE;

	VkDescriptorPool descriptor_pool_graphics = VK_NULL_HANDLE;
	vulkan_descriptor_set desc_set_0_graphics_output; // for rendering output image, buffer_graphics_camera + buffer_graphics_model_output
	vulkan_descriptor_set desc_set_1_graphics_output; // for rendering output image, texture_compute_output

	VkCommandPool command_pool_graphics = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer_graphics = VK_NULL_HANDLE;

	vulkan_mesh mesh_sprite;

	VkFence fence_submit_graphics = VK_NULL_HANDLE;

	int Init(VulkanApp* app);

	int CreateGraphicsPipeline();
	int CreateGraphicsDescriptorSets();
	int CreateGraphicsResources();
	int BindGraphicsAndComputeResources();
	int CreateGraphicsCommandBuffers();
	int CreateGraphicsSyncObjects();

	int Render(float dTime);
	int Submit();

	int CreatePlane();

	void Terminate();
};