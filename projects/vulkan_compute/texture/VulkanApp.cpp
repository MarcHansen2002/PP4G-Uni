#include "VulkanApp.h"

int VulkanApp::Init()
{
    // Compute
    if (!CreateContext())
    {
        DBG_ASSERT(false);
        return -1;
    }

    Smoke.CreateBuffers(physical_device, device);

    Compute.Init(this);

    // Setup smoke buffer
    if (!create_vulkan_buffer(
        physical_device,
        device,
        sizeof(float) * CellCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        buffer_density));

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

    Graphics.Init(this);
    imgui_setup_context(Graphics.GetCommandBuffer());
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
    if (!get_vulkan_queue_compute (*Compute.GetQueue()))
    {
      DBG_ASSERT (false);
      return -1;
    }
    if (!get_vulkan_queue_graphics (*Graphics.GetQueue()))
    {
      DBG_ASSERT (false);
      return -1;
    }
}

// Graphics

#include <sstream>

// Render & Shutdown
int VulkanApp::Render(float dTime)
{
    Emitter.Emit(Smoke, dTime);
    if (CompareSmokes)
    {
        Emitter.Emit(Smoke_CPU, dTime);
    }

    Compute.BindComputeResources();

    UpdateSmoke(dTime);

    Graphics.Render(dTime);
    Graphics.Submit();

    return 1;
}

void VulkanApp::RenderImGui(float dTime)
{
    ImGui::Begin("Test window");

    ImGui::Text("FPS: %i", (int)(dTime / 60.f));
    ImGui::NewLine();

    ImGui::Text("Smoke Parameters");
    ImGui::SliderFloat("Fluid Density", &FluidDensity, 0.001f, 100.f);
    ImGui::Checkbox("Run Compute", &RunCompute);
    if (ImGui::Checkbox("Compare Smokes", &CompareSmokes))
    {
        if (CompareSmokes)
        {
            Smoke.ClearSmoke();
            Smoke_CPU.ClearSmoke();
        }
    }

    // TODO: Smoke params
    ImGui::NewLine();
    ImGui::Text("Emitter Parameters");

    Emitter.ShowInspector();

    if (ImGui::Button("Clear Smoke"))
    {
        Smoke.ClearSmoke();
    }
    // TODO: Emitter params
    ImGui::End();
}

void VulkanApp::Terminate()
{
    imgui_shutdown();
    Graphics.Terminate();
    Compute.Terminate();
    
    release_vulkan_swapchain();
    release_vulkan_device();
    release_vulkan_surface();
    release_vulkan_instance();
    release_window();
    
    render_pass = VK_NULL_HANDLE;
    extent = {};
    device = VK_NULL_HANDLE;
    physical_device = VK_NULL_HANDLE;
}

