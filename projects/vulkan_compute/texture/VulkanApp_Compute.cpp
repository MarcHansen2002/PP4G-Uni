#include "VulkanApp_Compute.h"
#include "VulkanApp.h"

int VulkanApp_Compute::Init(VulkanApp* app)
{
    VkApp = app;

    if (!CreateComputePipeline())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!CreateComputeDescriptorSets())
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
    // TODO: I think this is the one that does the funky voodoo math
    // Might need to move this geezer since we need to compute each frame
    if (!ComputeDispatch())
    {
        DBG_ASSERT(false);
        return -1;
    }
}

int VulkanApp_Compute::CreateComputePipeline()
{
    // describe the descriptors in set 0
    std::array <VkDescriptorSetLayoutBinding, 4u> const descriptor_set_layout_binding_info_0 =
    {
        VkDescriptorSetLayoutBinding {
          .binding = 0u,          // VelocityX Binding
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // SBO (float array)
          .descriptorCount = 1u,
          .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
          .pImmutableSamplers = VK_NULL_HANDLE
        },
        VkDescriptorSetLayoutBinding {
            .binding = 1u,         // VelocityY Binding
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, // SBO (float array)
            .descriptorCount = 1u,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
        },
        VkDescriptorSetLayoutBinding {
            .binding = 2u,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1u,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
        },
        VkDescriptorSetLayoutBinding {
            .binding = 3u,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1u,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
        }
    };
    // group up all descriptor layout descriptions
    std::array <std::span <const VkDescriptorSetLayoutBinding>, NUM_SETS_COMPUTE> const descriptor_set_layout_bindings =
    {
      descriptor_set_layout_binding_info_0 // set 0
                                           // set 1...
    };
    if (!create_vulkan_descriptor_set_layouts(
        *VkApp->GetDevice(),
        NUM_SETS_COMPUTE,
        descriptor_set_layout_bindings.data(),
        descriptor_set_layouts_compute.data()))
    {
        DBG_ASSERT(false);
        return -1;
    }

    VkPushConstantRange const push_constant_ranges[] =
    {
        {
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
            .offset = 0u,
            .size = sizeof(smoke_push_constants_compute)
        }
    };

    if (!create_vulkan_pipeline_layout(
        *VkApp->GetDevice(),
        descriptor_set_layouts_compute.size(), descriptor_set_layouts_compute.data(),
        1u, push_constant_ranges,
        pipeline_layout_compute))
    {
        DBG_ASSERT(false);
        return -1;
    }

    // fill in compute shader 'bin/data/shaders/glsl/vulkan_compute_texture/vulkan_compute_texture.comp'
    // compile shaders by running 'compile_shaders_glsl.bat' (from the command line so you can check there are no errors)

    // TODO: Make shaders for all the other smoke simulation steps
    VkShaderModule shader_module_advect_velocity = VK_NULL_HANDLE;
    if (!create_vulkan_shader(
        *VkApp->GetDevice(),
        COMPILED_COMPUTE_SHADER_ADVECT_VELOCITY,
        shader_module_advect_velocity))
    {
        DBG_ASSERT(false);
        return -1;
    }

    if (!create_vulkan_pipeline_compute(*VkApp->GetDevice(),
        shader_module_advect_velocity, "main",
        pipeline_layout_compute,
        pipeline_compute))
    {
        DBG_ASSERT(false);
        return -1;
    }

    // No longer need shader objects once they're in the pipeline
    release_vulkan_shader(*VkApp->GetDevice(), shader_module_advect_velocity);
}

int VulkanApp_Compute::CreateComputeDescriptorSets()
{
    {
        // create pool of descriptors from which our descriptor sets will draw from
        //
        // we could have multiple descriptor pools, or one huge one
        // we are going to have one per pipeline in order to keep it simple
        std::array <VkDescriptorPoolSize, 1u> const pool_sizes =
        { { // yes this is deliberate!
            {
                  .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                  .descriptorCount = 4u
            }
            // if you also needed n uniform buffer descriptors you would need to create a new VkDescriptorPoolSize here
          } };
        if (!create_vulkan_descriptor_pool(
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
                // set 0
                {
                  .desc_pool = &descriptor_pool_compute,         // the pool from which to allocate the individual descriptors from
                  .layout = &descriptor_set_layouts_compute[0], // layout of the descriptor set
                  .set_index = 0u,                               // index of the descriptor set
                  .out_set = &desc_set_0_compute                 // pointer to where to instantiate the descriptor set to
                }
            // set 1...
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
    {
        VkDescriptorBufferInfo const buffer_infos[] =
        {
            {
                .buffer = VkApp->GetSmoke()->Buffer_VelocityX.buffer,
                .offset = 0u,
                .range = VK_WHOLE_SIZE
            },
            {
                .buffer = VkApp->GetSmoke()->Buffer_VelocityY.buffer,
                .offset = 0u,
                .range = VK_WHOLE_SIZE
            },
            {
                .buffer = VkApp->GetSmoke()->Buffer_Temp_VelocityX.buffer,
                .offset = 0u,
                .range = VK_WHOLE_SIZE
            },
            {
                .buffer = VkApp->GetSmoke()->Buffer_Temp_VelocityY.buffer,
                .offset = 0u,
                .range = VK_WHOLE_SIZE
            }
        };

        VkWriteDescriptorSet const write_descriptors[4] =
        {
            // desc_set_0_compute
            {
              .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              //.pNext = VK_NULL_HANDLE,
              .dstSet = desc_set_0_compute.desc_set,
              .dstBinding = 0u,
              .dstArrayElement = 0u,
              .descriptorCount = 1u,
              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
              .pImageInfo = VK_NULL_HANDLE, // Velocity X
              .pBufferInfo = &buffer_infos[0],
              .pTexelBufferView = VK_NULL_HANDLE
            },
            {
              .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              //.pNext = VK_NULL_HANDLE,
              .dstSet = desc_set_0_compute.desc_set,
              .dstBinding = 0u,
              .dstArrayElement = 0u,
              .descriptorCount = 1u,
              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
              .pImageInfo = VK_NULL_HANDLE, // Velocity Y
              .pBufferInfo = &buffer_infos[1],
              .pTexelBufferView = VK_NULL_HANDLE
            },
            {
              .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              //.pNext = VK_NULL_HANDLE,
              .dstSet = desc_set_0_compute.desc_set,
              .dstBinding = 0u,
              .dstArrayElement = 0u,
              .descriptorCount = 1u,
              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
              .pImageInfo = VK_NULL_HANDLE, // Velocity X (TEMP)
              .pBufferInfo = &buffer_infos[2],
              .pTexelBufferView = VK_NULL_HANDLE
            },
            {
              .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              //.pNext = VK_NULL_HANDLE,
              .dstSet = desc_set_0_compute.desc_set,
              .dstBinding = 0u,
              .dstArrayElement = 0u,
              .descriptorCount = 1u,
              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
              .pImageInfo = VK_NULL_HANDLE, // Velocity Y (TEMP)
              .pBufferInfo = &buffer_infos[3],
              .pTexelBufferView = VK_NULL_HANDLE
            },
        };

        vkUpdateDescriptorSets(*VkApp->GetDevice(), // device
            NUM_RESOURCES_COMPUTE_SET_0,  // descriptorWriteCount
            write_descriptors,            // pDescriptorWrites
            0u,                           // descriptorCopyCount
            VK_NULL_HANDLE);              // pDescriptorCopies
    }
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

int VulkanApp_Compute::ComputeDispatch()
{
    if (!map_and_unmap_memory(
        *VkApp->GetDevice(),
        VkApp->GetSmoke()->Buffer_VelocityX.memory, [this](void* mem)
        {
            memcpy(mem, VkApp->GetSmoke()->VelocityX, sizeof(float) * CellCount);
        }
    ))
    {
        assert(false);
        return -1;
    }
    if (!map_and_unmap_memory(
        *VkApp->GetDevice(),
        VkApp->GetSmoke()->Buffer_VelocityY.memory, [this](void* mem)
        {
            memcpy(mem, VkApp->GetSmoke()->VelocityY, sizeof(float) * CellCount);
        }
    ))
    {
        assert(false);
        return -1;
    }
    if (!map_and_unmap_memory(
        *VkApp->GetDevice(),
        VkApp->GetSmoke()->Buffer_Temp_VelocityY.memory, [this](void* mem)
        {
            memcpy(mem, VkApp->GetSmoke()->TempVelocityX, sizeof(float) * CellCount);
        }
    ))
    {
        assert(false);
        return -1;
    }
    if (!map_and_unmap_memory(
        *VkApp->GetDevice(),
        VkApp->GetSmoke()->Buffer_Temp_VelocityY.memory, [this](void* mem)
        {
            memcpy(mem, VkApp->GetSmoke()->TempVelocityX, sizeof(float) * CellCount);
        }
    ))
    {
        assert(false);
        return -1;
    }

    if (!begin_command_buffer(command_buffer_compute, 0u))
    {
        assert(false);
        return -1;
    }

    vkCmdBindPipeline(command_buffer_compute, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_compute);
    vkCmdBindDescriptorSets(command_buffer_compute, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout_compute,
        desc_set_0_compute.set_index, 1, &desc_set_0_compute.desc_set, 0u, VK_NULL_HANDLE);

    smoke_push_constants_compute const data =
    { 
        .GridSizeX = GridSizeX,
        .GridSizeY = GridSizeY,
        .CellCount = (u32)CellCount,
        .CellSize = CellSize,
    };

    vkCmdPushConstants(command_buffer_compute, pipeline_layout_compute, 0u, 0u, sizeof(smoke_push_constants_compute),
        &data);

    u32 const thread_group_dim = 256u; // Set in shader

    vkCmdDispatch(command_buffer_compute, 256, 1, 1);
    if (!end_command_buffer(command_buffer_compute))
    {
        assert(false);
        return -1;
    }

    vkResetFences(*VkApp->GetDevice(), 1, &fence_compute);

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
        return -1;
    }

    return 1;
    {
        // RECORD COMMAND BUFFER
        //{
        //    if (!begin_command_buffer(command_buffer_compute, 0u))
        //    {
        //        DBG_ASSERT(false);
        //        return -1;
        //    }
        //    {
        //        // any compute related command after this point is attached to this pipeline (on this command buffer)
                // bind compute pipeline
        //        vkCmdBindPipeline(
        //            command_buffer_compute,
        //            VK_PIPELINE_BIND_POINT_COMPUTE,
        //            pipeline_compute
        //        );
        //
        //        // bind descriptor set - texture_compute_input & texture_compute_output
        //        vkCmdBindDescriptorSets(
        //            command_buffer_compute,
        //            VK_PIPELINE_BIND_POINT_COMPUTE,
        //            pipeline_layout_compute,
        //            desc_set_0_compute.set_index,
        //            1,
        //            &desc_set_0_compute.desc_set,
        //            0u,
        //            VK_NULL_HANDLE
        //        );
        //
        //        // dispatch compute shader
                // see 'bin/data/shaders/glsl/vulkan_compute_texture/vulkan_compute_texture.comp' for thread group size!
                // HINT: the thread groups are 2D and so will the 'grid' of thread groups we want to create
        //        u32 const thread_group_dim = 16u;
        //        u32 const thread_group_count_x = (u32)std::ceilf((float)GridSizeX / (float)thread_group_dim);
        //        u32 const thread_group_count_y = (u32)std::ceilf((float)GridSizeY / (float)thread_group_dim);
        //        vkCmdDispatch(
        //            command_buffer_compute,
        //            thread_group_count_x,
        //            thread_group_count_y,
        //            1
        //        );
        //
        //    }
        //    if (!end_command_buffer(command_buffer_compute))
        //    {
        //        DBG_ASSERT(false);
        //        return -1;
        //    }
        //}
        //
        //// SUBMIT
        //{
        //    if (!CHECK_VULKAN_RESULT(vkResetFences(device, 1u, &fence_compute)))
        //    {
        //        DBG_ASSERT(false);
        //        return -1;
        //    }
        //
        //
        //    // submit compute commands
        //    VkSubmitInfo const submit_info =
        //    {
        //      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        //      //.pNext = VK_NULL_HANDLE,
        //      .waitSemaphoreCount = 0u,
        //      .pWaitSemaphores = VK_NULL_HANDLE,
        //      .pWaitDstStageMask = VK_NULL_HANDLE,
        //      .commandBufferCount = 1u,
        //      .pCommandBuffers = &command_buffer_compute,
        //      .signalSemaphoreCount = 0u,
        //      .pSignalSemaphores = VK_NULL_HANDLE
        //    };
        //
        //    if (!CHECK_VULKAN_RESULT(vkQueueSubmit(queue_compute, 1u, &submit_info,
        //        fence_compute))) // fence to signal when complete
        //    {
        //        DBG_ASSERT(false);
        //        return -1;
        //    }
        //
        //
        //    // wait for fence to be triggered via vkQueueSubmit (slow!)
        //    if (!CHECK_VULKAN_RESULT(vkWaitForFences(device,
        //        1u,
        //        &fence_compute,
        //        VK_TRUE,
        //        UINT64_MAX)))
        //    {
        //        DBG_ASSERT(false);
        //        return -1;
        //    }
        //}

        // TRANSITION IMAGE LAYOUTS
        {
            // NOTE: get output image ready for rendering via fragment shader
            // can only perform this now compute write pipeline has completed
            // TODO: once again, do we need this? We probably do just not here. Probably where we copy
            // The smoke data to the texture
            //if (!transition_image_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Graphics.render_texture))
            //{
            //    DBG_ASSERT(false);
            //    return -1;
            //}
            //if (!transition_image_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, Graphics.render_texture))
            //{
            //    DBG_ASSERT(false);
            //    return -1;
            //}

            // NOTE: this function is overloaded
            // the above version performs the transition as a 'one off' outside a command buffer
            // the other overload will add the required commands to an EXISTING command buffer
        }
    }
}

void VulkanApp_Compute::Terminate()
{
    release_vulkan_fences(1u, &fence_compute);

    release_vulkan_command_buffers(1u, command_pool_compute, &command_buffer_compute);
    release_vulkan_command_pool(command_pool_compute);

    //release_vulkan_texture(device, texture_compute_output);
    //release_vulkan_texture(device, texture_compute_input);

    release_vulkan_descriptor_sets(*VkApp->GetDevice(),
        1u, &desc_set_0_compute);
    release_vulkan_descriptor_pool(*VkApp->GetDevice(), descriptor_pool_compute);

    release_vulkan_pipeline(*VkApp->GetDevice(), pipeline_compute);
    release_vulkan_pipeline_layout(*VkApp->GetDevice(), pipeline_layout_compute);
    release_vulkan_descriptor_set_layouts(*VkApp->GetDevice(),
        NUM_SETS_COMPUTE, descriptor_set_layouts_compute.data());

    queue_compute = VK_NULL_HANDLE;
}