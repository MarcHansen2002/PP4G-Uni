#pragma once

#include "../maths.h"            // for standard types, glm::ortho
#include "../utility.h"          // for DBG_ASSERT
#include "../vulkan_context.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_resources.h"
#include "SmokeSimulation/SmokeData.h"

#include "imgui.h"

#include <array>                  // for std::array
#include <cmath>                  // for std::ceilf
#include <span>                   // for std::span

#define VK_USE_PLATFORM_WIN32_KHR // tell vulkan we are on Windows platform
#include <vulkan/vulkan.h>        // for everything vulkan
#define WIN32_LEAN_AND_MEAN       // exclude rarely-used content from the Windows headers
#define NOMINMAX                  // prevent Windows macros defining their own min and max macros
#include <Windows.h>              // for WinMain

#include "VulkanApp_Compute.h"
#include "VulkanApp_Graphics.h"

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
constexpr char const* COMPILED_COMPUTE_SHADER_PATH = "data/shaders/glsl/vulkan_compute_texture/vulkan_compute_texture.comp.spv";


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
	VulkanApp_Compute Compute;

	VkDevice* GetDevice() { return &device; }
	VkPhysicalDevice* GetPhysicalDevice() { return &physical_device; }
	VkRenderPass* GetRenderPass() { return &render_pass; }
	VkExtent2D* GetExtent() { return &extent; }

	vulkan_buffer* GetDensityBuffer() { return &buffer_density; }

	int Init();

	int CreateContext();

	int Render(float dTime);
	void RenderImGui(float dTime);

	void Terminate();
private:
	// TODO: Separate compute and graphics pipelines into their own classes for clarity
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkExtent2D extent = {};
	VkRenderPass render_pass = VK_NULL_HANDLE;

	vulkan_buffer buffer_density;				// For storing density to put into the previous image

	// ImGui
	VkDescriptorPool imgui_descriptor_pool;

	float elapsed = 0;
	
	// Could turn this into ImGui Editable
	void EmitSmoke(int x, int y, float density, float temp)
	{
		int i = Smoke.GetCellIndex(x, y);
		Smoke.Density[i] += density;
		Smoke.Temperature[i] += temp;

		// TODO: Temp testing values
		Smoke.VelocityY[Smoke.GetVelocityYIndex(x, y)] = -10.0f;
		Smoke.VelocityY[Smoke.GetVelocityYIndex(x, y + 1)] = -10.0f;
	}

	
	
	// Smoke Simulation
	// TODO: Move this into the compute shader
	void UpdateSmoke(float dTime)
	{
		if (dTime <= 0)
			return;

		Smoke.RunPressureSolver(&Smoke, dTime);
		Smoke.UpdateVelocities(&Smoke, dTime);
		Smoke.AdvectDye(&Smoke, dTime);;
		Smoke.AdvectVelocity(&Smoke, dTime);

		// TODO: (Smoke additions, do after moving current stuff to compute shader
		// Bouyancy
		// Velocity Dissipation
		// Density Dissipation
		// Velocity Diffusion
		// Vorticity Confinement
		// Temperature Dissipation
		// Obstacle interaction (very big stretch, not likely to happen)

		//SmokeIndex = 1 - SmokeIndex;
		// Re-copy memory to the GPU. This will not be needed once in the compute shader
		// As they will be modified on the GPU not on the CPU
		if (!map_and_unmap_memory(
			device,
			buffer_density.memory,
			[this](void* mem)
			{
				memcpy(mem, Smoke.Density, sizeof(float) * CellCount);
			}
		))
		{
			assert(false, "Unable to copy new density memory");
			return;
		}
	}

	SmokeState Smoke;
};