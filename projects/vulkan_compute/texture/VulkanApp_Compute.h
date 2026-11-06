#pragma once

#include "../maths.h"            // for standard types, glm::ortho
#include "../utility.h"          // for DBG_ASSERT
#include "../vulkan_context.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_resources.h"
#include "SmokeSimulation/SmokeData.h"
#include "Vulkan_Compute_Pipeline.h"

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
	int ComputeDispatch(float dTime);

	void Terminate();

	int UnmapData();

	VkDescriptorSetLayoutBinding MakeBinding
	(
		uint32_t binding,
		VkDescriptorType type,
		uint32_t count,
		VkShaderStageFlags flags,
		const VkSampler* sampler = VK_NULL_HANDLE
	);

	VkDescriptorSetLayout Layout_AdvectVelocity = VK_NULL_HANDLE;
	VkDescriptorSet Set_AdvectVelocity = VK_NULL_HANDLE;
	Vulkan_Compute_Pipeline Pipeline_AdvectVelocity;

	VkQueue queue_compute = VK_NULL_HANDLE;
	//std::array <VkDescriptorSetLayout, NUM_SETS_COMPUTE> descriptor_set_layouts_compute = { VK_NULL_HANDLE };
	VkDescriptorPool descriptor_pool_compute = VK_NULL_HANDLE;
	vulkan_descriptor_set DescriptorSet; //for compute, texture_compute_input + texture_compute_output

	VkCommandPool command_pool_compute = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer_compute = VK_NULL_HANDLE;

	VkFence fence_compute = VK_NULL_HANDLE;

	bool HasSubmit = false;
	char const* COMPILED_COMPUTE_SHADER_ADVECT_VELOCITY = "data/shaders/glsl/vulkan_compute_texture/Smoke/smoke_advect_velocity.comp.spv";

};