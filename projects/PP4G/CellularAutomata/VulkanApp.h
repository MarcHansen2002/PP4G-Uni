#pragma once

#include "../maths.h"            // for standard types, glm::ortho
#include "../utility.h"          // for DBG_ASSERT
#include "../vulkan_context.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_resources.h"

#include "imgui.h"

#include <array>                  // for std::array
#include <cmath>                  // for std::ceilf
#include <span>                   // for std::span

#define VK_USE_PLATFORM_WIN32_KHR // tell vulkan we are on Windows platform
#include <vulkan/vulkan.h>        // for everything vulkan
#define WIN32_LEAN_AND_MEAN       // exclude rarely-used content from the Windows headers
#define NOMINMAX                  // prevent Windows macros defining their own min and max macros
#include <Windows.h>              // for WinMain
#include <string>

#include "VulkanApp_Graphics.h"
#include "Automata/Automata.h"
#include "Automata/GameOfLife/GameOfLife.h"

struct vertex_2d_uv
{
	vec2 pos; // 2d pos
	vec2 uv;  // texture coordinates
};
struct camera_buffer
{
	mat4 vp_matrix;
};
struct model_buffer
{
	mat4 model_matrix;
};

struct smoke_render_buffer
{
	float density[CellCount];
};

constexpr char const* WINDOW_TITLE = "vulkan_compute_texture";

constexpr u32 NUM_RESOURCES_COMPUTE_SET_0 = 2u;
constexpr u32 BINDING_ID_SET_0_TEXTURE_INPUT = 0u;
constexpr u32 BINDING_ID_SET_0_TEXTURE_OUTPUT = 1u;



constexpr u32 NUM_RESOURCES_GRAPHICS_SET_0 = 2u;
constexpr u32 BINDING_ID_SET_0_UBO_CAMERA = 0u;
constexpr u32 BINDING_ID_SET_0_UBO_MODEL = 1u;

constexpr u32 NUM_RESOURCES_GRAPHICS_SET_1 = 1u;
constexpr u32 BINDING_ID_SET_1_TEXTURE = 0u;

class VulkanApp
{
public:
	VulkanApp()
	{
	}

	VulkanApp_Graphics Graphics;
	Automata* Grid = nullptr;

	VkDevice* GetDevice() { return &device; }
	VkPhysicalDevice* GetPhysicalDevice() { return &physical_device; }
	VkRenderPass* GetRenderPass() { return &render_pass; }
	VkExtent2D* GetExtent() { return &extent; }

	int Init();

	int CreateContext();

	int Render(float dTime);
	void RenderImGui(float dTime);

	void Terminate();
private:
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkExtent2D extent = {};
	VkRenderPass render_pass = VK_NULL_HANDLE;

	VkDescriptorPool imgui_descriptor_pool;

	bool play = false;

	GameTypes currentType = E_GameOfLife;

	void SwapType(GameTypes type)
	{
		if (Grid != nullptr)
		{
			Grid->Terminate(device);
			delete Grid;
			Grid = nullptr;
		}
		currentType = type;
		switch (currentType)
		{
		case E_GameOfLife:
			Grid = new GameOfLife;
			break;
		default:
			assert(false);
			return;
		}
		Grid->CreateBuffers(physical_device, device);
	}

};