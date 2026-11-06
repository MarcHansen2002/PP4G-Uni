#include "VulkanApp_Graphics.h"
#include "VulkanApp.h"

int VulkanApp_Graphics::Init(VulkanApp* app)
{
    VkApp = app;

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

int VulkanApp_Graphics::CreateGraphicsPipeline()
{
    {
        // describe the descriptors in set 0
        std::array <VkDescriptorSetLayoutBinding, 2> const descriptor_set_layout_binding_info_0 =
        {
            VkDescriptorSetLayoutBinding {
              .binding = 0,              // at binding point 0 we have
              .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // a uniform buffer
              .descriptorCount = 1u,
              .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
              .pImmutableSamplers = VK_NULL_HANDLE
            },
            VkDescriptorSetLayoutBinding {
              .binding = 1,               // at binding point 1 we have
              .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // another uniform buffer
              .descriptorCount = 1u,
              .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
              .pImmutableSamplers = VK_NULL_HANDLE
            }
        };
        // describe the descriptors in set 1
        std::array <VkDescriptorSetLayoutBinding, 1> const descriptor_set_layout_binding_info_1 =
        {
            VkDescriptorSetLayoutBinding {
              .binding = 0,                         // at binding point 1 we have
              .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // an image
              .descriptorCount = 1u,
              .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
              .pImmutableSamplers = VK_NULL_HANDLE
            }
        };
        // group up all descriptor layout descriptions
        std::array <std::span <const VkDescriptorSetLayoutBinding>, 2> const descriptor_set_layout_bindings =
        {
            descriptor_set_layout_binding_info_0, // set 0
            descriptor_set_layout_binding_info_1  // set 1
        };
        if (!create_vulkan_descriptor_set_layouts
        (
            *VkApp->GetDevice(),
            NUM_SETS_GRAPHICS,
            descriptor_set_layout_bindings.data(),
            DescriptorSetLayouts.data()
        ))
        {
            DBG_ASSERT(false);
            return -1;
        }

        if (!create_vulkan_pipeline_layout
        (
            *VkApp->GetDevice(),
            DescriptorSetLayouts.size(), DescriptorSetLayouts.data(),
            0u, nullptr,
            PipelineLayouts
        ))
        {
            DBG_ASSERT(false);
            return -1;
        }

        VkShaderModule shader_module_graphics_vert = VK_NULL_HANDLE;
        if (!create_vulkan_shader
        (
            *VkApp->GetDevice(),
            COMPILED_GRAPHICS_SHADER_PATH_VERT,
            shader_module_graphics_vert
        ))
        {
            DBG_ASSERT(false);
            return -1;
        }

        VkShaderModule shader_module_graphics_frag = VK_NULL_HANDLE;
        if (!create_vulkan_shader
        (
            *VkApp->GetDevice(),
            COMPILED_GRAPHICS_SHADER_PATH_FRAG,
            shader_module_graphics_frag
        ))
        {
            DBG_ASSERT(false);
            return -1;
        }


        VkViewport const viewport =
        {
            .x = 0.f,
            .y = 0.f,
            .width = (f32)VkApp->GetExtent()->width,
            .height = (f32)VkApp->GetExtent()->height,
            .minDepth = 0.f,
            .maxDepth = 1.f
        };
        VkRect2D const scissor =
        {
            .offset = { 0, 0 },
            .extent = *VkApp->GetExtent()
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

        if (!create_vulkan_pipeline_graphics
        (
            *VkApp->GetDevice(),
            shader_module_graphics_vert, "main",
            shader_module_graphics_frag, "main",
            viewport, scissor,
            pvisci,
            piasci,
            prsci,
            pmsci,
            pdssci,
            pcbsci,
            PipelineLayouts, *VkApp->GetRenderPass(), 0u,
            Pipeline
        ))
        {
            DBG_ASSERT(false);
            return -1;
        }

        // Don't need shader object now we have the pipeline
        release_vulkan_shader(*VkApp->GetDevice(), shader_module_graphics_frag); 
        release_vulkan_shader(*VkApp->GetDevice(), shader_module_graphics_vert);
    }
}

int VulkanApp_Graphics::CreateGraphicsDescriptorSets()
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

    if (!create_vulkan_descriptor_pool
    (
        *VkApp->GetDevice(),
        4u, // how many descriptor sets will we make from the sets in the pool?
        pool_sizes.size(), pool_sizes.data(),
        DescriptorPool
    ))
    {
        DBG_ASSERT(false);
        return -1;
    }
    
    std::array <vulkan_descriptor_set_info, NUM_SETS_GRAPHICS> const descriptor_set_infos =
    { {
        // set 0 - output
        {
            .desc_pool = &DescriptorPool,
            .layout = &DescriptorSetLayouts[0],
            .set_index = 0u,
            .out_set = &DescriptorSet_MVP
        },
        // set 1 - output
        {
            .desc_pool = &DescriptorPool,
            .layout = &DescriptorSetLayouts[1],
            .set_index = 1u,
            .out_set = &DescriptorSet_Texture
        }
    } };

    if (!create_vulkan_descriptor_sets
    (
        *VkApp->GetDevice(),
        descriptor_set_infos.size(),
        descriptor_set_infos.data()
    ))
    {
        DBG_ASSERT(false);
        return -1;
    }
}

int VulkanApp_Graphics::CreateGraphicsResources()
{
    {
        // NOTE: we are going to be reusing the textures made for the compute pipeline, so only need to make buffers

        // create vulkan buffer for 'buffer_graphics_camera'
        // create vulkan buffer for 'buffer_graphics_model_input'
        // create vulkan buffer for 'buffer_graphics_model_output'

        if (!create_vulkan_buffer
        (
            *VkApp->GetPhysicalDevice(),
            *VkApp->GetDevice(),
            sizeof(camera_buffer),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            Buffer_Camera
        ))
        {
            DBG_ASSERT(false);
            return -1;
        }

        if (!create_vulkan_buffer
        (
            *VkApp->GetPhysicalDevice(),
            *VkApp->GetDevice(),
            sizeof(model_buffer),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            Buffer_Plane
        ))
        {
            DBG_ASSERT(false);
            return -1;
        }


        // as the data will never change (because we are not actually double buffering and the camera/sprites will never move)
        // we can set the data in these buffers upfront

        if (!map_and_unmap_memory
        (
            *VkApp->GetDevice(),
            Buffer_Camera.memory,
            [this](void* mem)
            {
                camera_buffer* buf = (camera_buffer*)mem;

                // put camera origin in centre of screen

                f32 const screen_width_half = (f32)VkApp->GetExtent()->width / 2.f;
                f32 const screen_height_half = (f32)VkApp->GetExtent()->height / 2.f;

                f32 const left = -screen_width_half, right = screen_width_half;
                f32 const bottom = -screen_height_half, top = screen_height_half;
                f32 const z_near = 0.f, z_far = 1.f;

                buf->vp_matrix = glm::ortho(left, right, bottom, top, z_near, z_far);
                buf->vp_matrix[1][1] *= -1.f;
            }
        ))
        {
            DBG_ASSERT(false);
            return -1;
        }

        if (!map_and_unmap_memory
        (
            *VkApp->GetDevice(),
            Buffer_Plane.memory,
            [this](void* mem)
            {
                model_buffer* buf = (model_buffer*)mem;

                buf->model_matrix = fast_transform_2D
                (
                    0.f, 0.f,
                    0.f,
                    0.f, 0.f,
                    VkApp->GetExtent()->width, VkApp->GetExtent()->height
                );
            }
        ))
        {
            DBG_ASSERT(false);
            return -1;
        }


        VkFormat const image_format = VK_FORMAT_R8G8B8A8_UNORM;

        VkFormatProperties format_properties = {};
        // Get device properties for the requested texture format
        vkGetPhysicalDeviceFormatProperties(*VkApp->GetPhysicalDevice(), image_format, &format_properties);
        // NOTE: Check if requested image format supports image storage operations (VK_IMAGE_USAGE_STORAGE_BIT)
        if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) == 0u)
        {
            DBG_ASSERT_MSG(false, "device does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT\n");
            return -1;
        }

        const uvec2 ext = { GridSizeX, GridSizeY };
        if (!create_vulkan_texture_empty
        (
            *VkApp->GetPhysicalDevice(),
            *VkApp->GetDevice(),
            ext,
            image_format,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            Render_Texture
        ))
        {
            assert(false, "Input empty texture unable to create");
        }
    }
}

int VulkanApp_Graphics::BindGraphicsAndComputeResources()
{
    VkDescriptorBufferInfo const buffer_infos[] =
    {
        {
            .buffer = Buffer_Camera.buffer,
            .offset = 0u,
            .range = VK_WHOLE_SIZE
        },
        {
            .buffer = Buffer_Plane.buffer,
            .offset = 0u,
            .range = VK_WHOLE_SIZE
        }
    };
    VkDescriptorImageInfo const image_infos[1] =
    {
        {
            .sampler = Render_Texture.sampler,
            .imageView = Render_Texture.view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL // not in this format yet, but it will be!
        }
    };

    VkWriteDescriptorSet const write_descriptors[3] =
    {
        // desc_set_0_graphics_output - binding 0
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = VK_NULL_HANDLE,
            .dstSet = DescriptorSet_MVP.desc_set,
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
            .dstSet = DescriptorSet_MVP.desc_set,
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
            .dstSet = DescriptorSet_Texture.desc_set,
            .dstBinding = BINDING_ID_SET_1_TEXTURE,
            .dstArrayElement = 0u,
            .descriptorCount = 1u,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &image_infos[0],
            .pBufferInfo = VK_NULL_HANDLE,
            .pTexelBufferView = VK_NULL_HANDLE
        }
    };

    vkUpdateDescriptorSets
    (
        *VkApp->GetDevice(),          // device
        3u,                           // descriptorWriteCount
        write_descriptors,            // pDescriptorWrites
        0u,                           // descriptorCopyCount
        VK_NULL_HANDLE                // pDescriptorCopies
    );             

    return 1;
}

int VulkanApp_Graphics::CreateGraphicsCommandBuffers()
{
    if (!create_vulkan_command_pool(VK_QUEUE_GRAPHICS_BIT, CommandPool))
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!create_vulkan_command_buffers(1u, CommandPool, &CommandBuffer))
    {
        DBG_ASSERT(false);
        return -1;
    }
}

int VulkanApp_Graphics::CreateGraphicsSyncObjects()
{
    if (!create_vulkan_semaphores(1u, &swapchain_image_available_semaphore))
    {
        DBG_ASSERT(false);
        return -1;
    }
    if (!create_vulkan_fences(1u, 0u, &Fence))
    {
        DBG_ASSERT(false);
        return -1;
    }
}

int VulkanApp_Graphics::CreatePlane()
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

    if (!create_vulkan_mesh
    (
        *VkApp->GetPhysicalDevice(),
        *VkApp->GetDevice(),
        (void*)vertices, 4u, sizeof(vertex_2d_uv),
        (void*)indices, 6u, sizeof(u16),
        PlaneMesh
    ))
    {
        DBG_ASSERT(false);
        return -1;
    }
}


int VulkanApp_Graphics::Render(float dTime)
{
    // this semaphore will be triggered when a swapchain image is available to be rendered to
    if (!acquire_next_swapchain_image(swapchain_image_available_semaphore))
    {
        DBG_ASSERT(false);
        return -1;
    }

    // RECORD COMMAND BUFFER(S)
    {
        // we are reusing the graphics command buffer, so need to ensure it starts of empty
        if (!CHECK_VULKAN_RESULT(vkResetCommandBuffer(CommandBuffer, 0u)))
        {
            DBG_ASSERT(false);
            return -1;
        }

        if (!begin_command_buffer(CommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT))
        {
            DBG_ASSERT(false);
            return -1;
        }

        imgui_beginframe();

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

        vkCmdCopyBufferToImage
        (
            CommandBuffer,
            VkApp->GetDensityBuffer()->buffer,
            Render_Texture.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &copyRegion
        );


        begin_render_pass(CommandBuffer, { 0.39f, 0.8f, 0.92f }); // cornflower blue

        // any graphics related command after this point is attached to this pipeline (on this command buffer)
        // bind graphics pipeline

        vkCmdBindPipeline
        (
            CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            Pipeline
        );

        // render input image
        VkDeviceSize offsets = 0u; // use me for 'pOffsets' and 'offset'
        vkCmdBindVertexBuffers
        (
            CommandBuffer,
            0,
            1,
            &PlaneMesh.buffer_vertex,
            &offsets
        );
        vkCmdBindIndexBuffer
        (
            CommandBuffer,
            PlaneMesh.buffer_index,
            offsets,
            VK_INDEX_TYPE_UINT16
        );

        // vertices & indices are already bound!!!

        // bind graphics output descriptor set 0 - buffer_graphics_camera + buffer_graphics_model_output
        vkCmdBindDescriptorSets
        (
            CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PipelineLayouts,
            DescriptorSet_MVP.set_index,
            1,
            &DescriptorSet_MVP.desc_set,
            0u,
            VK_NULL_HANDLE
        );
        // bind graphics output descriptor set 1 - texture_compute_output
        vkCmdBindDescriptorSets
        (
            CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            PipelineLayouts,
            DescriptorSet_Texture.set_index,
            1,
            &DescriptorSet_Texture.desc_set,
            0u,
            VK_NULL_HANDLE
        );

        // TODO: call vkCmdDrawIndexed
        // we are drawing the same mesh again (just with different DSIs now :)
        vkCmdDrawIndexed
        (
            CommandBuffer,
            PlaneMesh.num_indices,
            1,
            0,
            0,
            0
        );

        VkApp->RenderImGui(dTime);
        imgui_endframe(&CommandBuffer);
        end_render_pass(CommandBuffer);


        if (!end_command_buffer(CommandBuffer))
        {
            DBG_ASSERT(false);
            return -1;
        }
    }
}

int VulkanApp_Graphics::Submit()
{
    if (!CHECK_VULKAN_RESULT(vkResetFences(*VkApp->GetDevice(), 1u, &Fence)))
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
        .pCommandBuffers = &CommandBuffer,
        .signalSemaphoreCount = 0u,
        .pSignalSemaphores = VK_NULL_HANDLE                     // semaphores to trigger when command buffer has finished executing
    };

    if (!CHECK_VULKAN_RESULT(vkQueueSubmit
    (
        Queue,
        1u,
        &submit_info,
        Fence
    ))) // fence to signal when complete
    {
        DBG_ASSERT(false);
        return -1;
    }


    // wait for fence to be triggered via vkQueueSubmit
    // would not do this here if 'properly' double buffering...
    // research it
    if (!CHECK_VULKAN_RESULT(vkWaitForFences
    (
        *VkApp->GetDevice(),
        1u,
        &Fence,
        VK_TRUE,
        UINT64_MAX
    )))
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

void VulkanApp_Graphics::Terminate()
{
    release_vulkan_fences(1u, &Fence);
    release_vulkan_semaphores(1u, &swapchain_image_available_semaphore);

    release_vulkan_mesh(*VkApp->GetDevice(), PlaneMesh);

    release_vulkan_command_buffers(1u, CommandPool, &CommandBuffer);

    release_vulkan_command_pool(CommandPool);

    release_vulkan_texture(*VkApp->GetDevice(), Render_Texture);

    release_vulkan_buffer(*VkApp->GetDevice(), Buffer_Plane);
    release_vulkan_buffer(*VkApp->GetDevice(), Buffer_Camera);

    release_vulkan_descriptor_sets(*VkApp->GetDevice(), 1u, &DescriptorSet_Texture);
    release_vulkan_descriptor_sets(*VkApp->GetDevice(), 1u, &DescriptorSet_MVP);
    release_vulkan_descriptor_pool(*VkApp->GetDevice(), DescriptorPool);

    release_vulkan_pipeline(*VkApp->GetDevice(), Pipeline);
    release_vulkan_pipeline_layout(*VkApp->GetDevice(), PipelineLayouts);
    release_vulkan_descriptor_set_layouts(*VkApp->GetDevice(), NUM_SETS_GRAPHICS, DescriptorSetLayouts.data());

    Queue = VK_NULL_HANDLE;
}