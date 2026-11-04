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
#include <string>

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
	SmokeState* GetSmoke() { return &Smoke; }

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

	// Smoke Settings
	float FluidDensity = 1.f;
	bool RunCompute = true;

	bool CompareSmokes = false;

	bool CheckArraySimilarity(float* A, float* B, int Count, std::string step, std::string arr, float Tolerance = 0.00001f)
	{
		for (int i = 0; i < Count; i++)
		{
			float diff = std::abs(A[i] - B[i]);
			if (diff > Tolerance)
			{
				assert(false);
				return false;
			}
		}
		return true;
	}

	bool CheckSmokeSimilarity(std::string step)
	{
		if (!CheckArraySimilarity(Smoke.Density, Smoke_CPU.Density, CellCount, step, "Density"))
		{
			assert(false);
			return false;
		}
		if (!CheckArraySimilarity(Smoke.VelocityX, Smoke_CPU.VelocityX, VeloSizeX, step, "VeloX"))
		{
			assert(false);
			return false;
		}
		if (!CheckArraySimilarity(Smoke.VelocityY, Smoke_CPU.VelocityY, VeloSizeY, step, "VeloY"))
		{
			assert(false);
			return false;
		}


		return true;
	}
	
	// Smoke Simulation
	// TODO: Move this into the compute shader
	void UpdateSmoke(float dTime)
	{
		if (dTime <= 0)
			return;

		if (CompareSmokes)
		{
			CheckSmokeSimilarity("Before update");

			Smoke_CPU.RunPressureSolver(FluidDensity, dTime);
			Smoke.RunPressureSolver(FluidDensity, dTime);
			// Compare
			CheckSmokeSimilarity("Pressure Solve");

			Smoke_CPU.UpdateVelocities(FluidDensity, dTime);
			Smoke.UpdateVelocities(FluidDensity, dTime);
			CheckSmokeSimilarity("Update Velocities");
			// Compare

			Smoke_CPU.AdvectDye(dTime);
			Smoke.AdvectDye(dTime);
			CheckSmokeSimilarity("Advect Dye");
			// Compare

			Smoke_CPU.AdvectVelocity(dTime);
			Compute.ComputeDispatch(dTime);

			CheckArraySimilarity(
				Smoke.TempVelocityX,
				Smoke_CPU.TempVelocityX,
				VeloSizeX,
				"Advect Velocity",
				"TempVelocityX");

			CheckArraySimilarity(
				Smoke.TempVelocityY,
				Smoke_CPU.TempVelocityY,
				VeloSizeY,
				"Advect Velocity",
				"TempVelocityY");

			for (int i = 0; i < (GridSizeX + 1) * GridSizeY; i++)
			{
				Smoke.VelocityX[i] = Smoke.TempVelocityX[i];
			}

			for (int i = 0; i < GridSizeX * (GridSizeY + 1); i++)
			{
				Smoke.VelocityY[i] = Smoke.TempVelocityY[i];
			}
			CheckSmokeSimilarity("Advect Velocity");
			// Compare
		}
		else if (RunCompute)
		{
			Smoke.RunPressureSolver(FluidDensity, dTime);
			Smoke.UpdateVelocities(FluidDensity, dTime);
			Smoke.AdvectDye(dTime);
			if (!Compute.ComputeDispatch(dTime))
			{
				assert(false);
				return;
			}

			

			for (int i = 0; i < (GridSizeX + 1) * GridSizeY; i++)
			{
				Smoke.VelocityX[i] = Smoke.TempVelocityX[i];
			}

			for (int i = 0; i < GridSizeX * (GridSizeY + 1); i++)
			{
				Smoke.VelocityY[i] = Smoke.TempVelocityY[i];
			}
		}
		else
		{
			Smoke.RunPressureSolver(FluidDensity, dTime);
			Smoke.UpdateVelocities(FluidDensity, dTime);
			Smoke.AdvectDye(dTime);
			Smoke.AdvectVelocity(dTime);
		}
		

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

	SmokeEmitter Emitter;
	SmokeState Smoke;
	SmokeState Smoke_CPU;
};