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
	void WriteDescriptorSet(VkDescriptorSetLayout& layout, const int count);
	int CreateComputeDescriptorSets();
	int BindComputeResource(vulkan_descriptor_set dstSet, std::vector<vulkan_buffer> buffers, const int count);
	int BindComputeResources();
	int CreateComputeCommandBuffers();
	int CreateComputeSyncObjects();
	int ComputeDispatch(float dTime);

	void WaitForBarrier(VkAccessFlags flags);

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

	uint32_t PipelineCount = 5;			// One per pipeline
	uint32_t DescriptorCount = 18;		// Total bindings in each pipeline


	VkDescriptorSetLayout Layout_PreparePressureSolver = VK_NULL_HANDLE;
	VkDescriptorSet Set_PreparePressureSolver = VK_NULL_HANDLE;
	Vulkan_Compute_Pipeline Pipeline_PreparePressureSolver;
	vulkan_descriptor_set DescriptorSet_PreparePressureSolver;

	VkDescriptorSetLayout Layout_PressureSolver = VK_NULL_HANDLE;
	VkDescriptorSet Set_PressureSolver = VK_NULL_HANDLE;
	Vulkan_Compute_Pipeline Pipeline_PressureSolver;
	vulkan_descriptor_set DescriptorSet_PressureSolver;
	vulkan_descriptor_set DescriptorSet_PressureSolverB;

	VkDescriptorSetLayout Layout_UpdateVelocities = VK_NULL_HANDLE;
	VkDescriptorSet Set_UpdateVelocities = VK_NULL_HANDLE;
	Vulkan_Compute_Pipeline Pipeline_UpdateVelocities;
	vulkan_descriptor_set DescriptorSet_UpdateVelocities;

	VkDescriptorSetLayout Layout_AdvectDye = VK_NULL_HANDLE;
	VkDescriptorSet Set_AdvectDye = VK_NULL_HANDLE;
	Vulkan_Compute_Pipeline Pipeline_AdvectDye;
	vulkan_descriptor_set DescriptorSet_AdvectDye;

	VkDescriptorSetLayout Layout_AdvectVelocity = VK_NULL_HANDLE;
	VkDescriptorSet Set_AdvectVelocity = VK_NULL_HANDLE;
	Vulkan_Compute_Pipeline Pipeline_AdvectVelocity;
	vulkan_descriptor_set DescriptorSet_AdvectVelocity;

	VkQueue queue_compute = VK_NULL_HANDLE;
	VkDescriptorPool descriptor_pool_compute = VK_NULL_HANDLE;

	VkCommandPool command_pool_compute = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer_compute = VK_NULL_HANDLE;

	VkFence fence_compute = VK_NULL_HANDLE;

	bool HasSubmit = false;
};