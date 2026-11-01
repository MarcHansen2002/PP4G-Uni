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

constexpr u32 NUM_SETS_COMPUTE = 1u;

class VulkanApp;

class VulkanApp_Compute
{
public:
	VulkanApp_Compute() {}

	VulkanApp* VkApp = VK_NULL_HANDLE;

	VkCommandBuffer* GetCommandBuffer() { return &command_buffer_compute; }
	VkQueue* GetQueue() { return &queue_compute; }
	VkFence* GetFence() { return &fence_compute; }

	int Init(VulkanApp* app);

	int CreateComputePipeline();
	int CreateComputeDescriptorSets();
	int CreateComputeResources();
	int BindComputeResources();
	int CreateComputeCommandBuffers();
	int CreateComputeSyncObjects();
	int ComputeDispatch();

	void Terminate();


	VkQueue queue_compute = VK_NULL_HANDLE;
	std::array <VkDescriptorSetLayout, NUM_SETS_COMPUTE> descriptor_set_layouts_compute = { VK_NULL_HANDLE };
	VkPipelineLayout pipeline_layout_compute = VK_NULL_HANDLE;
	VkPipeline pipeline_compute = VK_NULL_HANDLE;
	VkDescriptorPool descriptor_pool_compute = VK_NULL_HANDLE;
	vulkan_descriptor_set desc_set_0_compute; //for compute, texture_compute_input + texture_compute_output

	//vulkan_texture texture_compute_input;
	//vulkan_texture texture_compute_output;

	VkCommandPool command_pool_compute = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer_compute = VK_NULL_HANDLE;

	VkFence fence_compute = VK_NULL_HANDLE;
};