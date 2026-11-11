#pragma once

#include "../maths.h"            // for standard types, glm::ortho
#include "../utility.h"          // for DBG_ASSERT
#include "../vulkan_context.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_resources.h"

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

	VkSemaphore* GetComputeSemaphore() { return &semaphore_compute; }

	VulkanApp* VkApp = VK_NULL_HANDLE;
#pragma region Getters
	VkCommandBuffer* GetCommandBuffer() { return &CommandBuffer; }
	VkQueue* GetQueue() { return &Queue; }
	VkSemaphore* GetSemaphore() { return &swapchain_image_available_semaphore; }
	VkFence* GetFence() { return &Fence; }
#pragma endregion
	
#pragma region Members
	vulkan_buffer Buffer_Camera;	// Buffer for the view projection
	vulkan_buffer Buffer_Plane;		// Buffer for the plane mesh

	vulkan_texture Render_Texture;	// Empty texture to build the smoke density into

	VkSemaphore swapchain_image_available_semaphore = VK_NULL_HANDLE;
	VkSemaphore semaphore_compute = VK_NULL_HANDLE;

	VkQueue Queue = VK_NULL_HANDLE;

	std::array <VkDescriptorSetLayout, NUM_SETS_GRAPHICS> DescriptorSetLayouts = { VK_NULL_HANDLE, VK_NULL_HANDLE };

	VkPipelineLayout PipelineLayouts = VK_NULL_HANDLE;
	VkPipeline Pipeline = VK_NULL_HANDLE;

	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	vulkan_descriptor_set DescriptorSet_MVP;		// For rendering - Camera + Mesh
	vulkan_descriptor_set DescriptorSet_Texture;	// For Rendering - Smoke Texture

	VkCommandPool CommandPool = VK_NULL_HANDLE;
	VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;

	vulkan_mesh PlaneMesh;

	VkFence Fence = VK_NULL_HANDLE;
#pragma endregion

#pragma region Methods
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
#pragma endregion
};