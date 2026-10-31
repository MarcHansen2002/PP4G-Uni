#include <wrl/client.h>
#include <memory>
#include "VulkanApp.h"
#include <chrono>


int WINAPI WinMain (_In_ HINSTANCE/* hInstance*/,
  _In_opt_ HINSTANCE/* hPrevInstance*/,
  _In_ LPSTR/* lpCmdLine*/,
  _In_ int/* nShowCmd*/)
{
    std::chrono::high_resolution_clock clock;
    float dTime = 0.f;
    std::unique_ptr<VulkanApp> app = std::make_unique<VulkanApp>();

    app->Init();

    while (process_os_messages ())
    {
        auto start = clock.now();

        app->Render(dTime);
        app->Submit();

        auto end = clock.now();
        dTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }
    
    app->Terminate();
    return 0;
}
