#include <wrl/client.h>
#include <memory>
#include "VulkanApp.h"



int WINAPI WinMain (_In_ HINSTANCE/* hInstance*/,
  _In_opt_ HINSTANCE/* hPrevInstance*/,
  _In_ LPSTR/* lpCmdLine*/,
  _In_ int/* nShowCmd*/)
{
    std::unique_ptr<VulkanApp> app = std::make_unique<VulkanApp>();

    app->Init();

    while (process_os_messages ())
    {
        app->Render();
        app->Submit();
    }
    
    app->Terminate();
    return 0;
}
