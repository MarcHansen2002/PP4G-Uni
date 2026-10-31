#include "VulkanApp.h"

int VulkanApp::Init()
{
    // Compute
    if (!CreateContext())
    {
        DBG_ASSERT(false);
        return -1;
    }
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
    if (!ComputeDispatch())
    {
        DBG_ASSERT(false);
        return -1;
    }
    // Graphics
    if (!CreateGraphicsPipeline())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!CreateGraphicsDescriptorSets())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!CreateGraphicsResources())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!BindGraphicsAndComputeResources())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!CreateGraphicsCommandBuffers())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!CreateGraphicsSyncObjects())
    {
        DBG_ASSERT(false);
        return -1;
    }

    if (!CreatePlane())
    {
        DBG_ASSERT(false);
        return -1;
    }
}

// Compute
int VulkanApp::CreateContext()
{
    // create window, vulkan instance & device etc
    if (!create_window(WINDOW_TITLE))
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!create_vulkan_instance())
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!create_vulkan_surface ())
    {
      DBG_ASSERT (false);
      return -1;
    }
    if (!create_vulkan_device (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT,
      physical_device, device))
    {
      DBG_ASSERT (false);
      return -1;
    }
    if (!create_vulkan_swapchain (extent, render_pass))
    {
      DBG_ASSERT (false);
      return -1;
    }
    // get vulkan queues
    if (!get_vulkan_queue_compute (queue_compute))
    {
      DBG_ASSERT (false);
      return -1;
    }
    if (!get_vulkan_queue_graphics (queue_graphics))
    {
      DBG_ASSERT (false);
      return -1;
    }
}

int VulkanApp::CreateComputePipeline()
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
    if (!create_vulkan_descriptor_set_layouts(device,
        NUM_SETS_COMPUTE,
        descriptor_set_layout_bindings.data(),
        descriptor_set_layouts_compute.data()))
    {
        DBG_ASSERT(false);
        return -1;
    }
    
    if (!create_vulkan_pipeline_layout(device,
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
    if (!create_vulkan_shader(device,
        COMPILED_COMPUTE_SHADER_PATH,
        shader_module_compute))
    {
        DBG_ASSERT(false);
        return -1;
    }
    
    if (!create_vulkan_pipeline_compute(device,
        shader_module_compute, "main",
        pipeline_layout_compute,
        pipeline_compute))
    {
        DBG_ASSERT(false);
        return -1;
    }
    
    release_vulkan_shader(device,
        shader_module_compute); // don't need shader object now we have the pipeline
}

int VulkanApp::CreateComputeDescriptorSets()
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
        if (!create_vulkan_descriptor_pool(device,
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
        if (!create_vulkan_descriptor_sets(device,
            descriptor_set_infos.size(), descriptor_set_infos.data()))
        {
            DBG_ASSERT(false);
            return -1;
        }
    }
}

int VulkanApp::CreateComputeResources()
{
    {
        VkFormat const image_format = VK_FORMAT_R8G8B8A8_UNORM;

        VkFormatProperties format_properties = {};
        // Get device properties for the requested texture format
        vkGetPhysicalDeviceFormatProperties(physical_device, image_format, &format_properties);
        // NOTE: Check if requested image format supports image storage operations (VK_IMAGE_USAGE_STORAGE_BIT)
        if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) == 0u)
        {
            DBG_ASSERT_MSG(false, "device does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT\n");
            return -1;
        }

        const uvec2 ext = { GridSizeX, GridSizeY };
        // TODO: Remove this, but first we need to change the compute shader to stop needing this (only needs the smoke data arrays)
        if (!create_vulkan_texture_empty(
            physical_device,
            device,
            ext,
            image_format,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            texture_compute_input))
        {
            assert(false, "Input empty texture unable to create");
        }

        

        // call create_vulkan_texture_empty to create 'texture_compute_output'
        // needs to have the exact same size as texture_compute_input
        // needs general layout so it can be written to
        // usage = sampled and storage
        //   sampled = image can be sampled from later
        //   storage = so we can use it in the compute shader
        if (!create_vulkan_texture_empty(
            physical_device,
            device,
            ext,
            image_format,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            texture_compute_output))
        {
            assert(false, "Vulkan empty texture not created.");
        }
    }
}

int VulkanApp::BindComputeResources()
{
    {
        // desc_set_0_compute = for compute = texture_compute_input + texture_compute_output

        VkDescriptorImageInfo const image_infos[NUM_RESOURCES_COMPUTE_SET_0] =
        {
          {
            .sampler = texture_compute_input.sampler,
            .imageView = texture_compute_input.view,
            .imageLayout = texture_compute_input.layout
          },
          {
            .sampler = texture_compute_output.sampler,
            .imageView = texture_compute_output.view,
            .imageLayout = texture_compute_output.layout
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

        vkUpdateDescriptorSets(device, // device
            NUM_RESOURCES_COMPUTE_SET_0,  // descriptorWriteCount
            write_descriptors,            // pDescriptorWrites
            0u,                           // descriptorCopyCount
            VK_NULL_HANDLE);              // pDescriptorCopies
    }
    return 1;
}

int VulkanApp::CreateComputeCommandBuffers()
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

int VulkanApp::CreateComputeSyncObjects()
{
    {
        if (!create_vulkan_fences(1u, 0u, &fence_compute))
        {
            DBG_ASSERT(false);
            return -1;
        }
    }
}

int VulkanApp::ComputeDispatch()
{
    {
        // RECORD COMMAND BUFFER
        {
            if (!begin_command_buffer(command_buffer_compute, 0u))
            {
                DBG_ASSERT(false);
                return -1;
            }
            {
                // any compute related command after this point is attached to this pipeline (on this command buffer)
                // bind compute pipeline
                vkCmdBindPipeline(
                    command_buffer_compute,
                    VK_PIPELINE_BIND_POINT_COMPUTE,
                    pipeline_compute
                );

                // bind descriptor set - texture_compute_input & texture_compute_output
                vkCmdBindDescriptorSets(
                    command_buffer_compute,
                    VK_PIPELINE_BIND_POINT_COMPUTE,
                    pipeline_layout_compute,
                    desc_set_0_compute.set_index,
                    1,
                    &desc_set_0_compute.desc_set,
                    0u,
                    VK_NULL_HANDLE
                );

                // dispatch compute shader
                // see 'bin/data/shaders/glsl/vulkan_compute_texture/vulkan_compute_texture.comp' for thread group size!
                // HINT: the thread groups are 2D and so will the 'grid' of thread groups we want to create
                u32 const thread_group_dim = 16u;
                u32 const thread_group_count_x = (u32)std::ceilf((float)texture_compute_input.dim.x / (float)thread_group_dim);
                u32 const thread_group_count_y = (u32)std::ceilf((float)texture_compute_input.dim.y / (float)thread_group_dim);
                vkCmdDispatch(
                    command_buffer_compute,
                    thread_group_count_x,
                    thread_group_count_y,
                    1
                );

            }
            if (!end_command_buffer(command_buffer_compute))
            {
                DBG_ASSERT(false);
                return -1;
            }
        }

        // SUBMIT
        {
            if (!CHECK_VULKAN_RESULT(vkResetFences(device, 1u, &fence_compute)))
            {
                DBG_ASSERT(false);
                return -1;
            }


            // submit compute commands
            VkSubmitInfo const submit_info =
            {
              .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
              //.pNext = VK_NULL_HANDLE,
              .waitSemaphoreCount = 0u,
              .pWaitSemaphores = VK_NULL_HANDLE,
              .pWaitDstStageMask = VK_NULL_HANDLE,
              .commandBufferCount = 1u,
              .pCommandBuffers = &command_buffer_compute,
              .signalSemaphoreCount = 0u,
              .pSignalSemaphores = VK_NULL_HANDLE
            };

            if (!CHECK_VULKAN_RESULT(vkQueueSubmit(queue_compute, 1u, &submit_info,
                fence_compute))) // fence to signal when complete
            {
                DBG_ASSERT(false);
                return -1;
            }


            // wait for fence to be triggered via vkQueueSubmit (slow!)
            if (!CHECK_VULKAN_RESULT(vkWaitForFences(device,
                1u,
                &fence_compute,
                VK_TRUE,
                UINT64_MAX)))
            {
                DBG_ASSERT(false);
                return -1;
            }
        }

        // TRANSITION IMAGE LAYOUTS
        {
            // NOTE: get output image ready for rendering via fragment shader
            // can only perform this now compute write pipeline has completed
            if (!transition_image_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texture_compute_input))
            {
                DBG_ASSERT(false);
                return -1;
            }
            if (!transition_image_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texture_compute_output))
            {
                DBG_ASSERT(false);
                return -1;
            }

            // NOTE: this function is overloaded
            // the above version performs the transition as a 'one off' outside a command buffer
            // the other overload will add the required commands to an EXISTING command buffer
        }
    }
}

// Graphics
int VulkanApp::CreateGraphicsPipeline()
{
    {
        // describe the descriptors in set 0
        std::array <VkDescriptorSetLayoutBinding, NUM_RESOURCES_GRAPHICS_SET_0> const descriptor_set_layout_binding_info_0 =
        {
          VkDescriptorSetLayoutBinding {
            .binding = BINDING_ID_SET_0_UBO_CAMERA,              // at binding point 0 we have
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // a uniform buffer
            .descriptorCount = 1u,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
          },
          VkDescriptorSetLayoutBinding {
            .binding = BINDING_ID_SET_0_UBO_MODEL,               // at binding point 1 we have
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // another uniform buffer
            .descriptorCount = 1u,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
          }
        };
        // describe the descriptors in set 1
        std::array <VkDescriptorSetLayoutBinding, NUM_RESOURCES_GRAPHICS_SET_1> const descriptor_set_layout_binding_info_1 =
        {
          VkDescriptorSetLayoutBinding {
            .binding = BINDING_ID_SET_1_TEXTURE,                         // at binding point 1 we have
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // an image
            .descriptorCount = 1u,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = VK_NULL_HANDLE
          }
        };
        // group up all descriptor layout descriptions
        std::array <std::span <const VkDescriptorSetLayoutBinding>, NUM_SETS_GRAPHICS> const descriptor_set_layout_bindings =
        {
          descriptor_set_layout_binding_info_0, // set 0
          descriptor_set_layout_binding_info_1  // set 1
        };
        if (!create_vulkan_descriptor_set_layouts(device,
            NUM_SETS_GRAPHICS,
            descriptor_set_layout_bindings.data(),
            descriptor_set_layouts_graphics.data()))
        {
            DBG_ASSERT(false);
            return -1;
        }

        if (!create_vulkan_pipeline_layout(device,
            descriptor_set_layouts_graphics.size(), descriptor_set_layouts_graphics.data(),
            0u, nullptr,
            pipeline_layout_graphics))
        {
            DBG_ASSERT(false);
            return -1;
        }

        VkShaderModule shader_module_graphics_vert = VK_NULL_HANDLE;
        if (!create_vulkan_shader(device,
            COMPILED_GRAPHICS_SHADER_PATH_VERT,
            shader_module_graphics_vert))
        {
            DBG_ASSERT(false);
            return -1;
        }

        VkShaderModule shader_module_graphics_frag = VK_NULL_HANDLE;
        if (!create_vulkan_shader(device,
            COMPILED_GRAPHICS_SHADER_PATH_FRAG,
            shader_module_graphics_frag))
        {
            DBG_ASSERT(false);
            return -1;
        }


        VkViewport const viewport =
        {
          .x = 0.f,
          .y = 0.f,
          .width = (f32)extent.width,
          .height = (f32)extent.height,
          .minDepth = 0.f,
          .maxDepth = 1.f
        };
        VkRect2D const scissor =
        {
          .offset = { 0, 0 },
          .extent = extent
        };


        VkVertexInputBindingDescription vertex_input_binding_descriptions[] =
        {
          {
            .binding = 0u,
            .stride = sizeof(vertex_2d_uv),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
          }
        };

        VkVertexInputAttributeDescription vertex_input_attribute_descriptions[] =
        {
            // position
            {
              .location = 0u,
              .binding = 0u,
              .format = VK_FORMAT_R32G32_SFLOAT,
              .offset = 0u
            },
            // tex_coord
            {
              .location = 1u,
              .binding = 0u,
              .format = VK_FORMAT_R32G32_SFLOAT,
              .offset = sizeof(f32) * 2u
            }
        };

        VkPipelineVertexInputStateCreateInfo const pvisci =
        {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
          //.pNext = VK_NULL_HANDLE,
          //.flags = 0u,
          .vertexBindingDescriptionCount = 1u,
          .pVertexBindingDescriptions = vertex_input_binding_descriptions,
          .vertexAttributeDescriptionCount = 2u,
          .pVertexAttributeDescriptions = vertex_input_attribute_descriptions
        };

        VkPipelineInputAssemblyStateCreateInfo const piasci =
        {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
          //.pNext = VK_NULL_HANDLE,
          //.flags = 0u,
          .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
          .primitiveRestartEnable = VK_FALSE
        };

        VkPipelineRasterizationStateCreateInfo const prsci =
        {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
          //.pNext = VK_NULL_HANDLE,
          //.flags = 0u,
          .depthClampEnable = VK_FALSE,
          .rasterizerDiscardEnable = VK_FALSE,
          .polygonMode = VK_POLYGON_MODE_FILL,
          .cullMode = VK_CULL_MODE_BACK_BIT,
          .frontFace = VK_FRONT_FACE_CLOCKWISE,
          .depthBiasEnable = VK_FALSE,
          .depthBiasConstantFactor = 0.f,
          .depthBiasClamp = 0.f,
          .depthBiasSlopeFactor = 0.f,
          .lineWidth = 1.f
        };

        VkPipelineMultisampleStateCreateInfo const pmsci =
        {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
          //.pNext = VK_NULL_HANDLE,
          //.flags = 0u,
          .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
          .sampleShadingEnable = VK_FALSE,
          .minSampleShading = 0.f,
          .pSampleMask = nullptr,
          .alphaToCoverageEnable = VK_FALSE,
          .alphaToOneEnable = VK_FALSE
        };

        VkPipelineDepthStencilStateCreateInfo const pdssci =
        {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
          //.pNext = VK_NULL_HANDLE,
          //.flags = 0u,
          .depthTestEnable = VK_FALSE,
          .depthWriteEnable = VK_FALSE,
          .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
          .depthBoundsTestEnable = VK_FALSE,
          .stencilTestEnable = VK_FALSE,
          .minDepthBounds = 0.f,
          .maxDepthBounds = 1.f
        };

        VkPipelineColorBlendAttachmentState const pipeline_colour_blend_attachment_states[] =
        {
          {
    #if 0
            .blendEnable = VK_TRUE,

            // if you want alpha blending
            // colour.rgb = (srcColor * SRC_ALPHA) ADD (dstColor * ONE_MINUS_SRC_ALPHA)
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    #else
            .blendEnable = VK_FALSE,

            // colour.rgb = (srcColor * ONE) ADD (dstColor * ZERO)
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    #endif

            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT
              | VK_COLOR_COMPONENT_G_BIT
              | VK_COLOR_COMPONENT_B_BIT
              | VK_COLOR_COMPONENT_A_BIT
          }
        };
        VkPipelineColorBlendStateCreateInfo const pcbsci =
        {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
          //.pNext = VK_NULL_HANDLE,
          //.flags = 0u,
          .logicOpEnable = VK_FALSE,
          .logicOp = VK_LOGIC_OP_NO_OP,
          .attachmentCount = 1u,
          .pAttachments = pipeline_colour_blend_attachment_states,
          .blendConstants = { 0.f, 0.f, 0.f, 0.f }
        };

        if (!create_vulkan_pipeline_graphics(device,
            shader_module_graphics_vert, "main",
            shader_module_graphics_frag, "main",
            viewport, scissor,
            pvisci,
            piasci,
            prsci,
            pmsci,
            pdssci,
            pcbsci,
            pipeline_layout_graphics, render_pass, 0u,
            pipeline_graphics))
        {
            DBG_ASSERT(false);
            return -1;
        }

        release_vulkan_shader(device,
            shader_module_graphics_frag); // don't need shader object now we have the pipeline
        release_vulkan_shader(device,
            shader_module_graphics_vert);
    }
}

int VulkanApp::CreateGraphicsDescriptorSets()
{
    // to support n-buffering, each frame must have it's own instances of the descriptor sets, each with their own resources
    // i.e. resources (should) not be used simultaneously across frames
    // we are double buffering framebuffers, because Vulkan forces us to...
    // BUT, we are syncing between each frame to keep things simple, so one instance is enough

    // create pool of descriptors from which our descriptor sets will draw from
    //
    // we could have multiple descriptor pools, or one huge one
    // we are going to have one per pipeline in order to keep it simple
    std::array <VkDescriptorPoolSize, 2u> const pool_sizes =
    { { // yes this is deliberate!
      {
            // we have 2 x descriptors set that consist of 2 x uniform buffers
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 4u
          },
          {
              // we have 2 x descriptor set that consist of 1 x image descriptor
              .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              .descriptorCount = 2u
            }
          } };
    if (!create_vulkan_descriptor_pool(device,
        4u, // how many descriptor sets will we make from the sets in the pool?
        pool_sizes.size(), pool_sizes.data(),
        descriptor_pool_graphics))
    {
        DBG_ASSERT(false);
        return -1;
    }

    std::array <vulkan_descriptor_set_info, NUM_SETS_GRAPHICS /** 2u*/> const descriptor_set_infos =
    { {
        // set 0 - output
      {
        .desc_pool = &descriptor_pool_graphics,
        .layout = &descriptor_set_layouts_graphics[0],
        .set_index = 0u,
        .out_set = &desc_set_0_graphics_output
      },
        // set 1 - output
      {
          .desc_pool = &descriptor_pool_graphics,
          .layout = &descriptor_set_layouts_graphics[1],
          .set_index = 1u,
          .out_set = &desc_set_1_graphics_output
      }

        // fill in 'vulkan_descriptor_set_info' for desc_set_0_graphics_output
        // fill in 'vulkan_descriptor_set_info' for desc_set_1_graphics_output
      } };
    if (!create_vulkan_descriptor_sets(device,
        descriptor_set_infos.size(), descriptor_set_infos.data()))
    {
        DBG_ASSERT(false);
        return -1;
    }
}

int VulkanApp::CreateGraphicsResources()
{
    {
        // NOTE: we are going to be reusing the textures made for the compute pipeline, so only need to make buffers

        // create vulkan buffer for 'buffer_graphics_camera'
        // create vulkan buffer for 'buffer_graphics_model_input'
        // create vulkan buffer for 'buffer_graphics_model_output'

        if (!create_vulkan_buffer(
            physical_device,
            device,
            sizeof(float) * CellCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            buffer_density
        ));

        if (!map_and_unmap_memory(
            device,
            buffer_density.memory,
            [this](void* mem)
            {
                memcpy(mem, Smoke.Density, sizeof(float) * CellCount);
            }
        ))
        {
            return -1;
        }

        if (!create_vulkan_buffer(
            physical_device,
            device,
            sizeof(camera_buffer),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            buffer_graphics_camera))
        {
            DBG_ASSERT(false);
            return -1;
        }

        if (!create_vulkan_buffer(
            physical_device,
            device,
            sizeof(model_buffer),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            buffer_graphics_model_output))
        {
            DBG_ASSERT(false);
            return -1;
        }


        // as the data will never change (because we are not actually double buffering and the camera/sprites will never move)
        // we can set the data in these buffers upfront

        if (!map_and_unmap_memory(device,
            buffer_graphics_camera.memory,
            [this](void* mem)
            {
                camera_buffer* buf = (camera_buffer*)mem;

                // put camera origin in centre of screen

                f32 const screen_width_half = (f32)extent.width / 2.f;
                f32 const screen_height_half = (f32)extent.height / 2.f;

                f32 const left = -screen_width_half, right = screen_width_half;
                f32 const bottom = -screen_height_half, top = screen_height_half;
                f32 const z_near = 0.f, z_far = 1.f;

                buf->vp_matrix = glm::ortho(left, right, bottom, top, z_near, z_far);
                buf->vp_matrix[1][1] *= -1.f;
            }))
        {
            DBG_ASSERT(false);
            return -1;
        }

        if (!map_and_unmap_memory(device,
            buffer_graphics_model_output.memory,
            [this](void* mem)
            {
                model_buffer* buf = (model_buffer*)mem;

                buf->model_matrix = fast_transform_2D(0.f, 0.f,
                    0.f,
                    0.f, 0.f,
                    extent.width, extent.height);
            }))
        {
            DBG_ASSERT(false);
            return -1;
        }
    }
}

int VulkanApp::BindGraphicsAndComputeResources()
{
    VkDescriptorBufferInfo const buffer_infos[] =
    {
        {
            .buffer = buffer_graphics_camera.buffer,
            .offset = 0u,
            .range = VK_WHOLE_SIZE
        },
        {
            .buffer = buffer_graphics_model_output.buffer,
            .offset = 0u,
            .range = VK_WHOLE_SIZE
        }
    };
    VkDescriptorImageInfo const image_infos[1]=
    {
      {
        .sampler = texture_compute_output.sampler,
        .imageView = texture_compute_output.view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // not in this format yet, but it will be!
      }
    };

    VkWriteDescriptorSet const write_descriptors[3]=
    {
        // desc_set_0_graphics_output - binding 0
        {
          .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .pNext = VK_NULL_HANDLE,
          .dstSet = desc_set_0_graphics_output.desc_set,
          .dstBinding = 0,
          .dstArrayElement = 0u,
          .descriptorCount = 1u,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pImageInfo = VK_NULL_HANDLE,
          .pBufferInfo = &buffer_infos[0], // buffer_graphics_camera
          .pTexelBufferView = VK_NULL_HANDLE
        },
        // desc_set_0_graphics_output - binding 1
        {
           .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
           .pNext = VK_NULL_HANDLE,
          .dstSet = desc_set_0_graphics_output.desc_set,
          .dstBinding = 1,
          .dstArrayElement = 0u,
          .descriptorCount = 1u,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .pImageInfo = VK_NULL_HANDLE,
          .pBufferInfo = &buffer_infos[1], // graphics_output
          .pTexelBufferView = VK_NULL_HANDLE
        },

        // desc_set_1_graphics_output - binding 0
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = VK_NULL_HANDLE,
           .dstSet = desc_set_1_graphics_output.desc_set,
           .dstBinding = BINDING_ID_SET_1_TEXTURE,
           .dstArrayElement = 0u,
           .descriptorCount = 1u,
           .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
           .pImageInfo = &image_infos[0],
           .pBufferInfo = VK_NULL_HANDLE,
           .pTexelBufferView = VK_NULL_HANDLE
        }
    };

    vkUpdateDescriptorSets(device, // device
        3u,                           // descriptorWriteCount
        write_descriptors,            // pDescriptorWrites
        0u,                           // descriptorCopyCount
        VK_NULL_HANDLE);              // pDescriptorCopies

    return 1;
}

int VulkanApp::CreateGraphicsCommandBuffers()
{
    if (!create_vulkan_command_pool(VK_QUEUE_GRAPHICS_BIT, command_pool_graphics))
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!create_vulkan_command_buffers(1u, command_pool_graphics, &command_buffer_graphics))
    {
        DBG_ASSERT(false);
        return -1;
    }
}

int VulkanApp::CreateGraphicsSyncObjects()
{
    if (!create_vulkan_semaphores(1u, &swapchain_image_available_semaphore))
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!create_vulkan_fences(1u, 0u, &fence_submit_graphics))
    {
        DBG_ASSERT(false);
        return -1;
    }
}

int VulkanApp::CreatePlane()
{
    vertex_2d_uv const vertices[] =
    {
      {.pos = { -.5f,  .5f }, .uv { 0.f, 0.f } }, // 0, top left
      {.pos = {  .5f,  .5f }, .uv { 1.f, 0.f } }, // 1, top right
      {.pos = { -.5f, -.5f }, .uv { 0.f, 1.f } }, // 2, bottom left
      {.pos = {  .5f, -.5f }, .uv { 1.f, 1.f } }, // 3, bottom right
    };
    u16 const indices[] = // clockwise order
    {
      0u, 1u, 2u, // face 0
      2u, 1u, 3u, // face 1
    };

    if (!create_vulkan_mesh(physical_device, device,
        (void*)vertices, 4u, sizeof(vertex_2d_uv),
        (void*)indices, 6u, sizeof(u16),
        mesh_sprite))
    {
        DBG_ASSERT(false);
        return -1;
    }
}
#include <sstream>

// Render & Shutdown
int VulkanApp::Render(float dTime)
{
    elapsed += dTime;
    if (elapsed >= 3.f)
    {
        EmitSmoke(GridSizeX / 2, GridSizeY / 2, 0.2f, 5.f);
        elapsed = 0;
    }

    
    UpdateSmoke(dTime);

        // this semaphore will be triggered when a swapchain image is available to be rendered to
        if (!acquire_next_swapchain_image(swapchain_image_available_semaphore))
        {
            DBG_ASSERT(false);
            return -1;
        }

        // RECORD COMMAND BUFFER(S)
        {
            // we are reusing the graphics command buffer, so need to ensure it starts of empty
            if (!CHECK_VULKAN_RESULT(vkResetCommandBuffer(command_buffer_graphics, 0u)))
            {
                DBG_ASSERT(false);
                return -1;
            }

            if (!begin_command_buffer(command_buffer_graphics, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT))
            {
                DBG_ASSERT(false);
                return -1;
            }

            VkBufferImageCopy copyRegion =
            {
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .imageOffset = {0, 0, 0},
                .imageExtent =
                {
                    GridSizeX,
                    GridSizeY,
                    1
                }
            };

            vkCmdCopyBufferToImage(
                command_buffer_graphics,
                buffer_density.buffer,
                texture_compute_output.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &copyRegion
            );


            begin_render_pass(command_buffer_graphics, { 0.39f, 0.8f, 0.92f }); // cornflower blue

            // any graphics related command after this point is attached to this pipeline (on this command buffer)
            // bind graphics pipeline

            vkCmdBindPipeline(
                command_buffer_graphics,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline_graphics
            );

            // render input image
            VkDeviceSize offsets = 0u; // use me for 'pOffsets' and 'offset'
            vkCmdBindVertexBuffers(
                command_buffer_graphics,
                0,
                1,
                &mesh_sprite.buffer_vertex,
                &offsets
            );
            vkCmdBindIndexBuffer(
                command_buffer_graphics,
                mesh_sprite.buffer_index,
                offsets,
                VK_INDEX_TYPE_UINT16
            );
            
            // vertices & indices are already bound!!!
            
            // bind graphics output descriptor set 0 - buffer_graphics_camera + buffer_graphics_model_output
            vkCmdBindDescriptorSets(command_buffer_graphics,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline_layout_graphics,
                desc_set_0_graphics_output.set_index,
                1,
                &desc_set_0_graphics_output.desc_set,
                0u,
                VK_NULL_HANDLE);
            // bind graphics output descriptor set 1 - texture_compute_output
            vkCmdBindDescriptorSets(command_buffer_graphics,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline_layout_graphics,
                desc_set_1_graphics_output.set_index,
                1,
                &desc_set_1_graphics_output.desc_set,
                0u,
                VK_NULL_HANDLE);
            
            // TODO: call vkCmdDrawIndexed
            // we are drawing the same mesh again (just with different DSIs now :)
            vkCmdDrawIndexed(
                command_buffer_graphics,
                mesh_sprite.num_indices,
                1,
                0,
                0,
                0
            );

            end_render_pass(command_buffer_graphics);


            if (!end_command_buffer(command_buffer_graphics))
            {
                DBG_ASSERT(false);
                return -1;
            }
        }
}

int VulkanApp::Submit()
{
    if (!CHECK_VULKAN_RESULT(vkResetFences(device, 1u, &fence_submit_graphics)))
    {
        DBG_ASSERT(false);
        return -1;
    }


    // submit graphics commands
    VkPipelineStageFlags const wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // look me up!
    // fill in 'submit_info'
    // remember we must WAIT for 'swapchain_image_available_semaphore' semaphore to be triggered before we submit
    VkSubmitInfo const submit_info =
    {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      //.pNext = VK_NULL_HANDLE,
      .waitSemaphoreCount = 1u,
      .pWaitSemaphores = &swapchain_image_available_semaphore, // wait for these semaphores to be signalled before submitting command buffers
      .pWaitDstStageMask = &wait_stage_mask,
      .commandBufferCount = 1u,
      .pCommandBuffers = &command_buffer_graphics,
      .signalSemaphoreCount = 0u,
      .pSignalSemaphores = VK_NULL_HANDLE                     // semaphores to trigger when command buffer has finished executing
    };

    if (!CHECK_VULKAN_RESULT(vkQueueSubmit(queue_graphics, 1u, &submit_info,
        fence_submit_graphics))) // fence to signal when complete
    {
        DBG_ASSERT(false);
        return -1;
    }


    // wait for fence to be triggered via vkQueueSubmit
    // would not do this here if 'properly' double buffering...
    // research it
    if (!CHECK_VULKAN_RESULT(vkWaitForFences(device,
        1u,
        &fence_submit_graphics,
        VK_TRUE,
        UINT64_MAX)))
    {
        DBG_ASSERT(false);
        return -1;
    }

    if (!present())
    {
        DBG_ASSERT(false);
        return -1;
    }
}

void VulkanApp::Terminate()
{
    release_vulkan_fences(1u, &fence_submit_graphics);
    release_vulkan_semaphores(1u, &swapchain_image_available_semaphore);
    
    release_vulkan_mesh(device, mesh_sprite);
    
    release_vulkan_command_buffers(1u, command_pool_graphics, &command_buffer_graphics);
    release_vulkan_command_pool(command_pool_graphics);
    
    release_vulkan_buffer(device, buffer_graphics_model_output);
    release_vulkan_buffer(device, buffer_graphics_camera);
    
    release_vulkan_descriptor_sets(device,
        1u, &desc_set_1_graphics_output);
    release_vulkan_descriptor_sets(device,
        1u, &desc_set_0_graphics_output);
    release_vulkan_descriptor_pool(device, descriptor_pool_graphics);
    
    release_vulkan_pipeline(device, pipeline_graphics);
    release_vulkan_pipeline_layout(device, pipeline_layout_graphics);
    release_vulkan_descriptor_set_layouts(device,
        NUM_SETS_GRAPHICS, descriptor_set_layouts_graphics.data());

    // COMPUTE PIPELINE
    release_vulkan_fences(1u, &fence_compute);
    
    release_vulkan_command_buffers(1u, command_pool_compute, &command_buffer_compute);
    release_vulkan_command_pool(command_pool_compute);
    
    release_vulkan_texture(device, texture_compute_output);
    release_vulkan_texture(device, texture_compute_input);
    
    release_vulkan_descriptor_sets(device,
        1u, &desc_set_0_compute);
    release_vulkan_descriptor_pool(device, descriptor_pool_compute);
    
    release_vulkan_pipeline(device, pipeline_compute);
    release_vulkan_pipeline_layout(device, pipeline_layout_compute);
    release_vulkan_descriptor_set_layouts(device,
        NUM_SETS_COMPUTE, descriptor_set_layouts_compute.data());

    // CONTEXT
    release_vulkan_swapchain();
    release_vulkan_device();
    release_vulkan_surface();
    release_vulkan_instance();
    release_window();
    queue_compute = queue_graphics = VK_NULL_HANDLE;
    render_pass = VK_NULL_HANDLE;
    extent = {};
    device = VK_NULL_HANDLE;
    physical_device = VK_NULL_HANDLE;
}

