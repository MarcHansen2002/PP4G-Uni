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
    if (!CreateComputeResources())
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

    //Set descriptors
    //Set Shader
    Pipeline_AdvectVelocity.CreatePipeline(*VkApp->GetDevice(), Layout_AdvectVelocity);
    Pipeline_AdvectVelocity.SetShader(*VkApp->GetDevice(), "data/shaders/glsl/vulkan_compute_texture/Smoke/smoke_advect_velocity.comp.spv");
    

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

int VulkanApp_Compute::CreateComputeDescriptorSets()
{
    // Advect Velocity
    std::array<VkDescriptorSetLayoutBinding, 4> const AdvectVelocityBindings =
    {
        MakeBinding(0u, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1u, VK_SHADER_STAGE_COMPUTE_BIT),    // Velocity X
        MakeBinding(1u, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1u, VK_SHADER_STAGE_COMPUTE_BIT),    // Velocity Y
        MakeBinding(2u, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1u, VK_SHADER_STAGE_COMPUTE_BIT),    // Temp Velocity X
        MakeBinding(3u, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1u, VK_SHADER_STAGE_COMPUTE_BIT),    // Temp Velocity Y
    };
    std::array <std::span <const VkDescriptorSetLayoutBinding>, 1> const AdvectVelocitySetBindings =
    {
      AdvectVelocityBindings
    };
    if (!create_vulkan_descriptor_set_layouts
    (
        *VkApp->GetDevice(),
        1u,
        AdvectVelocitySetBindings.data(),
        &Layout_AdvectVelocity
    ))
    {
        DBG_ASSERT(false);
        return -1;
    }
    // Apply velocity...
    // Velocity dissipation...
    // etc etc...

    {
        std::array <VkDescriptorPoolSize, 1u> const pool_sizes =
        { {
            {
                  .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                  .descriptorCount = 4u
            }
          } };

        if (!create_vulkan_descriptor_pool
        (
            *VkApp->GetDevice(),
            1u, // how many descriptor sets will we make from the sets in the pool?
            pool_sizes.size(), pool_sizes.data(),
            descriptor_pool_compute))
        {
            DBG_ASSERT(false);
            return -1;
        }

        std::array <vulkan_descriptor_set_info, NUM_SETS_COMPUTE> const descriptor_set_infos =
        { {
                // Advect Velocity
                {
                  .desc_pool = &descriptor_pool_compute,         // the pool from which to allocate the individual descriptors from
                  .layout = &Layout_AdvectVelocity, // layout of the descriptor set
                  .set_index = 0u,                               // index of the descriptor set
                  .out_set = &DescriptorSet // pointer to where to instantiate the descriptor set to
                }
            // The other pipelines
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

int VulkanApp_Compute::CreateComputeResources()
{
    return 1;
}

int VulkanApp_Compute::BindComputeResources()
{
    // Advect Velocity Resources
    SmokeState* smoke = VkApp->GetSmoke();
    VkDescriptorBufferInfo const AdvectVelocityBufferInfos[] =
    {
        {smoke->Buffer_VelocityX.buffer, 0u, VK_WHOLE_SIZE},
        {smoke->Buffer_VelocityY.buffer, 0u, VK_WHOLE_SIZE},
        {smoke->Buffer_Temp_VelocityX.buffer, 0u, VK_WHOLE_SIZE},
        {smoke->Buffer_Temp_VelocityY.buffer, 0u, VK_WHOLE_SIZE},
    };
    VkWriteDescriptorSet AdvectVelocityWriteDescriptors[4] =
    {
    };
    for (int i = 0; i < 4; i++)
    {
        AdvectVelocityWriteDescriptors[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        AdvectVelocityWriteDescriptors[i].dstSet = DescriptorSet.desc_set;
        AdvectVelocityWriteDescriptors[i].dstBinding = i;
        AdvectVelocityWriteDescriptors[i].descriptorCount = 1;
        AdvectVelocityWriteDescriptors[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        AdvectVelocityWriteDescriptors[i].pBufferInfo = &AdvectVelocityBufferInfos[i];
    }
    vkUpdateDescriptorSets
    (
        *VkApp->GetDevice(),                       // device
        4u,                                        // descriptorWriteCount
        AdvectVelocityWriteDescriptors,            // pDescriptorWrites
        0u,                                        // descriptorCopyCount
        VK_NULL_HANDLE
    );

    //{
    //    VkDescriptorBufferInfo const buffer_infos[] =
    //    {
    //        {
    //            .buffer = VkApp->GetSmoke()->Buffer_VelocityX.buffer,
    //            .offset = 0u,
    //            .range = VK_WHOLE_SIZE
    //        },
    //        {
    //            .buffer = VkApp->GetSmoke()->Buffer_VelocityY.buffer,
    //            .offset = 0u,
    //            .range = VK_WHOLE_SIZE
    //        },
    //        {
    //            .buffer = VkApp->GetSmoke()->Buffer_Temp_VelocityX.buffer,
    //            .offset = 0u,
    //            .range = VK_WHOLE_SIZE
    //        },
    //        {
    //            .buffer = VkApp->GetSmoke()->Buffer_Temp_VelocityY.buffer,
    //            .offset = 0u,
    //            .range = VK_WHOLE_SIZE
    //        }
    //    };
    //
    //    VkWriteDescriptorSet const write_descriptors[4] =
    //    {
    //        // desc_set_0_compute
    //        {
    //          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    //          //.pNext = VK_NULL_HANDLE,
    //          .dstSet = DescriptorSet.desc_set,
    //          .dstBinding = 0u,
    //          .dstArrayElement = 0u,
    //          .descriptorCount = 1u,
    //          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //          .pImageInfo = VK_NULL_HANDLE, // Velocity X
    //          .pBufferInfo = &buffer_infos[0],
    //          .pTexelBufferView = VK_NULL_HANDLE
    //        },
    //        {
    //          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    //          //.pNext = VK_NULL_HANDLE,
    //          .dstSet = DescriptorSet.desc_set,
    //          .dstBinding = 1u,
    //          .dstArrayElement = 0u,
    //          .descriptorCount = 1u,
    //          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //          .pImageInfo = VK_NULL_HANDLE, // Velocity Y
    //          .pBufferInfo = &buffer_infos[1],
    //          .pTexelBufferView = VK_NULL_HANDLE
    //        },
    //        {
    //          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    //          //.pNext = VK_NULL_HANDLE,
    //          .dstSet = DescriptorSet.desc_set,
    //          .dstBinding = 2u,
    //          .dstArrayElement = 0u,
    //          .descriptorCount = 1u,
    //          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //          .pImageInfo = VK_NULL_HANDLE, // Velocity X (TEMP)
    //          .pBufferInfo = &buffer_infos[2],
    //          .pTexelBufferView = VK_NULL_HANDLE
    //        },
    //        {
    //          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    //          //.pNext = VK_NULL_HANDLE,
    //          .dstSet = DescriptorSet.desc_set,
    //          .dstBinding = 3u,
    //          .dstArrayElement = 0u,
    //          .descriptorCount = 1u,
    //          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //          .pImageInfo = VK_NULL_HANDLE, // Velocity Y (TEMP)
    //          .pBufferInfo = &buffer_infos[3],
    //          .pTexelBufferView = VK_NULL_HANDLE
    //        },
    //    };
    //
    //    vkUpdateDescriptorSets
    //    (
    //        *VkApp->GetDevice(), // device
    //        4u,  // descriptorWriteCount
    //        write_descriptors,            // pDescriptorWrites
    //        0u,                           // descriptorCopyCount
    //        VK_NULL_HANDLE
    //    );              // pDescriptorCopies
    //}
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

    Pipeline_AdvectVelocity.Run(command_buffer_compute, dTime, DescriptorSet.set_index, &DescriptorSet.desc_set);

    u32 const thread_group_size = 16u; // Set in shader
    u32 const thread_group_dim_x = (GridSizeX + 1 + thread_group_size - 1) / thread_group_size;
    u32 const thread_group_dim_y = (GridSizeY + 1 + thread_group_size - 1) / thread_group_size;

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
    return 1;
}

void VulkanApp_Compute::Terminate()
{
    release_vulkan_fences(1u, &fence_compute);

    release_vulkan_command_buffers(1u, command_pool_compute, &command_buffer_compute);
    release_vulkan_command_pool(command_pool_compute);

    release_vulkan_descriptor_sets(*VkApp->GetDevice(),
        1u, &DescriptorSet);
    release_vulkan_descriptor_pool(*VkApp->GetDevice(), descriptor_pool_compute);

    Pipeline_AdvectVelocity.Release(*VkApp->GetDevice());

    release_vulkan_descriptor_set_layouts(*VkApp->GetDevice(), 1u, &Layout_AdvectVelocity);

   //release_vulkan_descriptor_set_layouts(*VkApp->GetDevice(),
   //    NUM_SETS_COMPUTE, descriptor_set_layouts_compute.data());

    queue_compute = VK_NULL_HANDLE;
}