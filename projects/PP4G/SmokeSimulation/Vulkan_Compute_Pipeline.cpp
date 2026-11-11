#include "Vulkan_Compute_Pipeline.h"

void Vulkan_Compute_Pipeline::SetShader(VkDevice dev, std::string path)
{
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (!create_vulkan_shader(
        dev,
        path.c_str(),
        shaderModule))
    {
        DBG_ASSERT(false);
        return;
    }

    if (!create_vulkan_pipeline_compute(
        dev,
        shaderModule, "main",
        layout,
        pipeline))
    {
        DBG_ASSERT(false);
        return;
    }

    // No longer need shader objects once they're in the pipeline
    release_vulkan_shader(dev, shaderModule);
}

void Vulkan_Compute_Pipeline::CreatePipeline(VkDevice dev, VkDescriptorSetLayout SetLayout)
{
    VkPushConstantRange const push_constant_ranges[] =
    {
        {
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .offset = 0u,
            .size = sizeof(smoke_push_constants_compute)
        }
    };

    if (!create_vulkan_pipeline_layout(
        dev,
        1u,
        &SetLayout,
        1u,
        push_constant_ranges,
        layout))
    {
        DBG_ASSERT(false);
        return;
    }
}

void Vulkan_Compute_Pipeline::Run(VkCommandBuffer buff, float dTime, u32 firstSet, const VkDescriptorSet* set)
{
    vkCmdBindPipeline
    (
        buff,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        pipeline
    );

    vkCmdBindDescriptorSets
    (
        buff,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        layout,
        firstSet,
        1,
        set,
        0u,
        VK_NULL_HANDLE
    );

    smoke_push_constants_compute const data =
    {
        .GridSizeX = GridSizeX,
        .GridSizeY = GridSizeY,
        .dTime = dTime,
        .CellSize = CellSize,
    };

    vkCmdPushConstants
    (
        buff,
        layout,
        VK_SHADER_STAGE_COMPUTE_BIT,
        0u,
        sizeof(smoke_push_constants_compute),
        &data
    );
}

void Vulkan_Compute_Pipeline::Release(VkDevice dev)
{
    release_vulkan_pipeline(dev, pipeline);
    release_vulkan_pipeline_layout(dev, layout);
}