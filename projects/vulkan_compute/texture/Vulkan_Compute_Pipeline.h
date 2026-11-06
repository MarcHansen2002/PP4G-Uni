#pragma once
#include <vulkan/vulkan.h>
#include "SmokeSimulation/SmokeData.h"
#include "../maths.h"            // for standard types, glm::ortho
#include "../utility.h"          // for DBG_ASSERT
#include "../vulkan_context.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_resources.h"
#include <string>

class Vulkan_Compute_Pipeline
{
public:
	//TODO:
	// SetShader
	// SetDescriptorSets
	// Checks to ensure it is setup before use
	void SetShader(VkDevice dev, std::string path);

	void CreatePipeline(VkDevice dev, VkDescriptorSetLayout SetLayout);

	void Run(VkCommandBuffer buff, float dTime, int firstSet, const VkDescriptorSet* set);

	void Release(VkDevice dev);

	VkPipeline GetPipeline() { return pipeline; }
	VkPipelineLayout GetLayout() { return layout; }

private:
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
};