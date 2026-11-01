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
    std::array <VkDescriptorSetLayoutBinding, NUM_RESOURCES_COMPUTE_SET_0> const descriptor_set_layout_binding_info_0 =
    {
      VkDescriptorSetLayoutBinding {
        .binding = BINDING_ID_SET_0_TEXTURE_INPUT,          // at binding point 0 we have
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, // an image (input)
        .descriptorCount = 1u,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .pImmutableSamplers = VK_NULL_HANDLE
      },
      VkDescriptorSetLayoutBinding {
        .binding = BINDING_ID_SET_0_TEXTURE_OUTPUT,         // at binding point 1 we have
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, // another image (output)
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
    if (!create_vulkan_descriptor_set_layouts(*VkApp->GetDevice(),
        NUM_SETS_COMPUTE,
        descriptor_set_layout_bindings.data(),
        descriptor_set_layouts_compute.data()))
    {
        DBG_ASSERT(false);
        return -1;
    }

    if (!create_vulkan_pipeline_layout(*VkApp->GetDevice(),
        descriptor_set_layouts_compute.size(), descriptor_set_layouts_compute.data(),
        0u, nullptr,
        pipeline_layout_compute))
    {
        DBG_ASSERT(false);
        return -1;
    }

    // fill in compute shader 'bin/data/shaders/glsl/vulkan_compute_texture/vulkan_compute_texture.comp'
    // compile shaders by running 'compile_shaders_glsl.bat' (from the command line so you can check there are no errors)

    VkShaderModule shader_module_compute = VK_NULL_HANDLE;
    if (!create_vulkan_shader(*VkApp->GetDevice(),
        COMPILED_COMPUTE_SHADER_PATH,
        shader_module_compute))
    {
        DBG_ASSERT(false);
        return -1;
    }

    if (!create_vulkan_pipeline_compute(*VkApp->GetDevice(),
        shader_module_compute, "main",
        pipeline_layout_compute,
        pipeline_compute))
    {
        DBG_ASSERT(false);
        return -1;
    }

    release_vulkan_shader(*VkApp->GetDevice(),
        shader_module_compute); // don't need shader object now we have the pipeline
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
                // we have 1 x descriptor set that consists of 2 x 'storage image' descriptors
                // A storage image (VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) is a descriptor type associated with an image
                // resource via an image view that load, store and atomic operations can be performed on.
                .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = 2u
              }
            // if you also needed n uniform buffer descriptors you would need to create a new VkDescriptorPoolSize here
          } };
        if (!create_vulkan_descriptor_pool(*VkApp->GetDevice(),
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
        if (!create_vulkan_descriptor_sets(*VkApp->GetDevice(),
            descriptor_set_infos.size(), descriptor_set_infos.data()))
        {
            DBG_ASSERT(false);
            return -1;
        }
    }
}

int VulkanApp_Compute::CreateComputeResources()
{
    //TODO: I don't think we need this anymore
    return 1;
}

int VulkanApp_Compute::BindComputeResources()
{
    {
        // desc_set_0_compute = for compute = texture_compute_input + texture_compute_output
        // TODO: Don't think we need this anymore but we'll set it just to make it happy because we're nice like that
        VkDescriptorImageInfo const image_infos[NUM_RESOURCES_COMPUTE_SET_0] =
        {
          {

                //TODO: Do we need this still?
                // Compute only really needs a bunch of float arrays as it's modifying the smoke data
                // Not a smoke texture
            .sampler = VkApp->Graphics.render_texture.sampler,
            .imageView = VkApp->Graphics.render_texture.view,
            .imageLayout = VkApp->Graphics.render_texture.layout
          },
          {
            .sampler = VkApp->Graphics.render_texture.sampler,
            .imageView = VkApp->Graphics.render_texture.view,
            .imageLayout = VkApp->Graphics.render_texture.layout
          }
        };
        VkWriteDescriptorSet const write_descriptors[NUM_RESOURCES_COMPUTE_SET_0] =
        {
            // desc_set_0_compute
            {
              .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              //.pNext = VK_NULL_HANDLE,
              .dstSet = desc_set_0_compute.desc_set,
              .dstBinding = BINDING_ID_SET_0_TEXTURE_INPUT,
              .dstArrayElement = 0u,
              .descriptorCount = 1u,
              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
              .pImageInfo = &image_infos[0], // texture_compute_input
              .pBufferInfo = VK_NULL_HANDLE,
              .pTexelBufferView = VK_NULL_HANDLE
            },
            // desc_set_0_compute
            {
              .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
              //.pNext = VK_NULL_HANDLE,
              .dstSet = desc_set_0_compute.desc_set,
              .dstBinding = BINDING_ID_SET_0_TEXTURE_OUTPUT,
              .dstArrayElement = 0u,
              .descriptorCount = 1u,
              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
              .pImageInfo = &image_infos[1], // texture_compute_output
              .pBufferInfo = VK_NULL_HANDLE,
              .pTexelBufferView = VK_NULL_HANDLE
            }
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