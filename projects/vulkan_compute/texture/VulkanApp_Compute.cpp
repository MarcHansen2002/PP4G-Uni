#include "VulkanApp_Compute.h"
#include "VulkanApp.h"

int VulkanApp_Compute::Init(VulkanApp* app)
{
    VkApp = app;

    if (!CreateComputeDescriptorSets())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!CreateComputePipeline())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!BindComputeResources())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!CreateComputeCommandBuffers())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!CreateComputeSyncObjects())
    {
        DBG_ASSERT(false);
        return -1;
    }
}

int VulkanApp_Compute::CreateComputePipeline()
{
    Pipeline_AdvectVelocity.CreatePipeline(*VkApp->GetDevice(), Layout_AdvectVelocity);
    Pipeline_AdvectVelocity.SetShader(*VkApp->GetDevice(), "data/shaders/glsl/vulkan_compute_texture/Smoke/smoke_advect_velocity.comp.spv");
    
    Pipeline_AdvectDye.CreatePipeline(*VkApp->GetDevice(), Layout_AdvectDye);
    Pipeline_AdvectDye.SetShader(*VkApp->GetDevice(), "data/shaders/glsl/vulkan_compute_texture/Smoke/smoke_advect_dye.comp.spv");
    //TODO: Other pipelines
    return 1;
}

VkDescriptorSetLayoutBinding VulkanApp_Compute::MakeBinding
(
    uint32_t binding,
    VkDescriptorType type,
    uint32_t count,
    VkShaderStageFlags flags,
    const VkSampler* sampler
)
{
    return VkDescriptorSetLayoutBinding{
        .binding = binding,
        .descriptorType = type,
        .descriptorCount = count,
        .stageFlags = flags,
        .pImmutableSamplers = sampler
    };
}

void VulkanApp_Compute::WriteDescriptorSet(VkDescriptorSetLayout& layout, const int count)
{
    std::vector< VkDescriptorSetLayoutBinding> Bindings(count);
    for (int i = 0; i < count; i++)
    {
        Bindings[i] = MakeBinding(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1u, VK_SHADER_STAGE_COMPUTE_BIT);
    }
    std::array <std::span <const VkDescriptorSetLayoutBinding>, 1> const SetBindings =
    {
        Bindings
    };
    if (!create_vulkan_descriptor_set_layouts
    (
        *VkApp->GetDevice(),
        1u,
        SetBindings.data(),
        &layout
    ))
    {
        DBG_ASSERT(false);
        return;
    }
}

int VulkanApp_Compute::CreateComputeDescriptorSets()
{
    // TODO: Pressure Solver
    // TODO: Update Velocities
    WriteDescriptorSet(Layout_AdvectDye, 4);
    WriteDescriptorSet(Layout_AdvectVelocity, 4);

    // Apply velocity...
    // Velocity dissipation...
    // etc etc...

    {
        std::array <VkDescriptorPoolSize, 1u> const pool_sizes =
        { {
            {
                  .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                  .descriptorCount = 8u
            }
          } };

        if (!create_vulkan_descriptor_pool
        (
            *VkApp->GetDevice(),
            2u, // how many descriptor sets will we make from the sets in the pool?
            pool_sizes.size(), pool_sizes.data(),
            descriptor_pool_compute))
        {
            DBG_ASSERT(false);
            return -1;
        }

        std::array <vulkan_descriptor_set_info, 2> const descriptor_set_infos =
        { {
            // Advect Velocity
            {
                .desc_pool = &descriptor_pool_compute,         // the pool from which to allocate the individual descriptors from
                .layout = &Layout_AdvectVelocity, // layout of the descriptor set
                .set_index = 0u,                               // index of the descriptor set
                .out_set = &DescriptorSet_AdvectVelocity // pointer to where to instantiate the descriptor set to
            },
            // Advect Dye
            {
                .desc_pool = &descriptor_pool_compute,
                .layout = &Layout_AdvectDye,
                .set_index = 0u,
                .out_set = &DescriptorSet_AdvectDye
            }
            // TODO: Pressure Solve
            // TODO: Update Velocities
          } };

        if (!create_vulkan_descriptor_sets(
            *VkApp->GetDevice(),
            descriptor_set_infos.size(), descriptor_set_infos.data()))
        {
            DBG_ASSERT(false);
            return -1;
        }
    }
}

int VulkanApp_Compute::BindComputeResource(vulkan_descriptor_set dstSet, std::vector<vulkan_buffer> buffers, const int count)
{
    std::vector<VkDescriptorBufferInfo> BufferInfos(count);
    std::vector<VkWriteDescriptorSet> WriteDescriptors(count);
    for (int i = 0; i < count; i++)
    {
        BufferInfos[i] = { buffers[i].buffer, 0, VK_WHOLE_SIZE };

        WriteDescriptors[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteDescriptors[i].dstSet = dstSet.desc_set;
        WriteDescriptors[i].dstBinding = i;
        WriteDescriptors[i].descriptorCount = 1;
        WriteDescriptors[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        WriteDescriptors[i].pBufferInfo = &BufferInfos[i];
    }

    vkUpdateDescriptorSets
    (
        *VkApp->GetDevice(),                       // device
        count,                                        // descriptorWriteCount
        WriteDescriptors.data(),            // pDescriptorWrites
        0u,                                        // descriptorCopyCount
        VK_NULL_HANDLE
    );

    return 1;
}

int VulkanApp_Compute::BindComputeResources()
{
    SmokeState* smoke = VkApp->GetSmoke();
    // TODO: Pressure Solver

    // TODO: Update Velocities

    // Advect Dye
    BindComputeResource
    (
        DescriptorSet_AdvectDye,
        { smoke->Buffer_VelocityX, smoke->Buffer_VelocityY, smoke->Buffer_Density, smoke->Buffer_Temp_Density },
        4
    );
    // Advect Velocity
    BindComputeResource
    (
        DescriptorSet_AdvectVelocity,
        {smoke->Buffer_VelocityX, smoke->Buffer_VelocityY, smoke->Buffer_Temp_VelocityX, smoke->Buffer_Temp_VelocityY},
        4
    );
    return 1;
}

int VulkanApp_Compute::CreateComputeCommandBuffers()
{
    {
        if (!create_vulkan_command_pool(VK_QUEUE_COMPUTE_BIT, command_pool_compute))
        {
            DBG_ASSERT(false);
            return -1;
        }
        if (!create_vulkan_command_buffers(1u, command_pool_compute, &command_buffer_compute))
        {
            DBG_ASSERT(false);
            return -1;
        }
    }
}

int VulkanApp_Compute::CreateComputeSyncObjects()
{
    {
        if (!create_vulkan_fences(1u, 0u, &fence_compute))
        {
            DBG_ASSERT(false);
            return -1;
        }
    }
}

int VulkanApp_Compute::ComputeDispatch(float dTime)
{
    if (!VkApp->GetSmoke()->MapBuffers(*VkApp->GetDevice()))
    {
        assert(false);
        return 0;
    }

    if (!begin_command_buffer(command_buffer_compute, 0u))
    {
        assert(false);
        return 0;
    }

    u32 const thread_group_size = 16u; // Set in shader
    u32 const thread_group_dim_x = (GridSizeX + 1 + thread_group_size - 1) / thread_group_size;
    u32 const thread_group_dim_y = (GridSizeY + 1 + thread_group_size - 1) / thread_group_size;

    // TODO: Pressure Solver

    // TODO: Update Velocities

    // TODO: Barrier will be needed here since updatevelocities writes to velocity whilst both advects read from it

    Pipeline_AdvectDye.Run(command_buffer_compute, dTime, DescriptorSet_AdvectDye.set_index, &DescriptorSet_AdvectDye.desc_set);
    vkCmdDispatch(command_buffer_compute, thread_group_dim_x, thread_group_dim_y, 1);
    //  ^^^ No barrier needed since they do not require anything the other writes vvv
    Pipeline_AdvectVelocity.Run(command_buffer_compute, dTime, DescriptorSet_AdvectVelocity.set_index, &DescriptorSet_AdvectVelocity.desc_set);
    vkCmdDispatch(command_buffer_compute, thread_group_dim_x, thread_group_dim_y, 1);

    VkMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;

    vkCmdPipelineBarrier(
        command_buffer_compute,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_HOST_BIT,
        0,
        1,
        &barrier,
        0, nullptr,
        0, nullptr
    );

    if (!end_command_buffer(command_buffer_compute))
    {
        assert(false);
        return 0;
    }

    if (HasSubmit)
    {
        if (vkResetFences(*VkApp->GetDevice(), 1, &fence_compute) != VK_SUCCESS)
        {
            assert(false);
            return 0;
        }
    }
    

    VkSubmitInfo const submit_info =
    {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      //.pNext = VK_NULL_HANDLE,
      .waitSemaphoreCount = 0u,
      .pWaitSemaphores = VK_NULL_HANDLE,          // wait for these semaphores to be signalled before submitting command buffers
      .pWaitDstStageMask = VK_NULL_HANDLE,
      .commandBufferCount = 1u,
      .pCommandBuffers = &command_buffer_compute,
      .signalSemaphoreCount = 0u,
      .pSignalSemaphores = VK_NULL_HANDLE         // semaphores to trigger when command buffer has finished executing
    };
    if (!CHECK_VULKAN_RESULT(vkQueueSubmit(queue_compute, 1u, &submit_info,
        fence_compute))) // fence to signal when complete
    {
        DBG_ASSERT(false);
        return 0;
    }
    else
    {
        HasSubmit = true;
    }

    if (vkWaitForFences(
        *VkApp->GetDevice(),
        1u,
        &fence_compute,
        true,
        UINT64_MAX
    ) != VK_SUCCESS)
    {
        assert(false);
        return 0;
    }

    if (!UnmapData())
    {
        assert(false);
        return 0;
    }

    return 1;
}

int VulkanApp_Compute::UnmapData()
{
    SmokeState* smoke = VkApp->GetSmoke();
    // TODO: Map more buffers as needed (pressure solve, update velocities etc)
    if (!map_and_unmap_memory(
        *VkApp->GetDevice(),
        smoke->Buffer_Temp_VelocityX.memory,
        [smoke](void* mem)
        {
            memcpy(smoke->VelocityX, mem, sizeof(float) * VeloSizeX);
        }
    ))
    {
        assert(false);
        return 0;
    }
    if (!map_and_unmap_memory(
        *VkApp->GetDevice(),
        smoke->Buffer_Temp_VelocityY.memory,
        [smoke](void* mem)
        {
            memcpy(smoke->VelocityY, mem, sizeof(float) * VeloSizeY);
        }
    ))
    {
        assert(false);
        return 0;
    }

    if (!map_and_unmap_memory(
        *VkApp->GetDevice(),
        smoke->Buffer_Temp_Density.memory,
        [smoke](void* mem)
        {
            memcpy(smoke->Density, mem, sizeof(float) * CellCount);
        }
    ))
    {
        assert(false);
        return 0;
    }

    return 1;
}

void VulkanApp_Compute::Terminate()
{
    release_vulkan_fences(1u, &fence_compute);

    release_vulkan_command_buffers(1u, command_pool_compute, &command_buffer_compute);
    release_vulkan_command_pool(command_pool_compute);

    // TODO: Pressure Solver
    // TODO: Update Velocities
    release_vulkan_descriptor_sets(*VkApp->GetDevice(), 1u, &DescriptorSet_AdvectVelocity);
    release_vulkan_descriptor_sets(*VkApp->GetDevice(), 1u, &DescriptorSet_AdvectDye);

    release_vulkan_descriptor_pool(*VkApp->GetDevice(), descriptor_pool_compute);

    Pipeline_AdvectVelocity.Release(*VkApp->GetDevice());
    Pipeline_AdvectDye.Release(*VkApp->GetDevice());

    release_vulkan_descriptor_set_layouts(*VkApp->GetDevice(), 1u, &Layout_AdvectVelocity);
    release_vulkan_descriptor_set_layouts(*VkApp->GetDevice(), 1u, &Layout_AdvectDye);

    queue_compute = VK_NULL_HANDLE;
}