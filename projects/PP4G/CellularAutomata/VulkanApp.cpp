#include "VulkanApp.h"
#include <random>

int VulkanApp::Init()
{
    // Compute
    if (!CreateContext())
    {
        DBG_ASSERT(false);
        return -1;
    }
    SwapType(E_GameOfLife);
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
    if (play)
    {
        Grid->UpdateOnce(simd);
    }
    Grid->MapBuffer(device);

    Graphics.Render(dTime);

    return 1;
}

void VulkanApp::RenderImGui(float dTime)
{
    ImGui::Begin("Test window");

    ImGui::Text("FPS: %.2f", (1000.f / dTime));
    ImGui::NewLine();
    ImGui::Checkbox("Use SIMD", &simd);
    ImGui::Checkbox("Play/Pause", &play);
    if (ImGui::Button("Proceed one frame"))
    {
        Grid->UpdateOnce(simd);
    }

    if (ImGui::Button("Clear Grid"))
    {
        std::fill_n(Grid->Cells, CellCount, 0);
    }
    if (ImGui::Button("Fill Random"))
    {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 1);

        std::generate_n(Grid->Cells, CellCount, [&]()
            {
                return float(dist(rng));
            });

    }

    Grid->Inspector();

    ImGui::End();
}

void VulkanApp::Terminate()
{
    imgui_shutdown();
    Graphics.Terminate();
    
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

