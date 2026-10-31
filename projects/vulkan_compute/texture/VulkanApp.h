#pragma once

#include "../maths.h"            // for standard types, glm::ortho
#include "../utility.h"          // for DBG_ASSERT
#include "../vulkan_context.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_resources.h"
#include "SmokeSimulation/SmokeData.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"


#include <array>                  // for std::array
#include <cmath>                  // for std::ceilf
#include <span>                   // for std::span

#define VK_USE_PLATFORM_WIN32_KHR // tell vulkan we are on Windows platform
#include <vulkan/vulkan.h>        // for everything vulkan
#define WIN32_LEAN_AND_MEAN       // exclude rarely-used content from the Windows headers
#define NOMINMAX                  // prevent Windows macros defining their own min and max macros
#include <Windows.h>              // for WinMain

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
constexpr char const* COMPILED_GRAPHICS_SHADER_PATH_VERT = "data/shaders/glsl/vulkan_compute_texture/sprite.vert.spv";
constexpr char const* COMPILED_GRAPHICS_SHADER_PATH_FRAG = "data/shaders/glsl/vulkan_compute_texture/sprite.frag.spv";
constexpr char const* TEXTURE_PATH = "data/textures/test.png";

constexpr u32 NUM_SETS_COMPUTE = 1u;

constexpr u32 NUM_RESOURCES_COMPUTE_SET_0 = 2u;
constexpr u32 BINDING_ID_SET_0_TEXTURE_INPUT = 0u;
constexpr u32 BINDING_ID_SET_0_TEXTURE_OUTPUT = 1u;

constexpr u32 NUM_SETS_GRAPHICS = 2u;

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

	int Init();

	int CreateContext();

	int CreateComputePipeline();
	int CreateComputeDescriptorSets();
	int CreateComputeResources();
	int BindComputeResources();
	int CreateComputeCommandBuffers();
	int CreateComputeSyncObjects();
	int ComputeDispatch();

	int CreateGraphicsPipeline();
	int CreateGraphicsDescriptorSets();
	int CreateGraphicsResources();
	int BindGraphicsAndComputeResources();
	int CreateGraphicsCommandBuffers();
	int CreateGraphicsSyncObjects();

	int CreatePlane();

	int Render(float dTime);
	int Submit();

	void Terminate();
private:
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkExtent2D extent = {};
	VkRenderPass render_pass = VK_NULL_HANDLE;

	VkQueue queue_compute = VK_NULL_HANDLE, queue_graphics = VK_NULL_HANDLE;

	std::array <VkDescriptorSetLayout, NUM_SETS_COMPUTE> descriptor_set_layouts_compute = { VK_NULL_HANDLE };
	VkPipelineLayout pipeline_layout_compute = VK_NULL_HANDLE;
	VkPipeline pipeline_compute = VK_NULL_HANDLE;

	VkDescriptorPool descriptor_pool_compute = VK_NULL_HANDLE;
	vulkan_descriptor_set desc_set_0_compute; //for compute, texture_compute_input + texture_compute_output

	vulkan_texture texture_compute_input;
	vulkan_texture texture_compute_output;

	VkCommandPool command_pool_compute = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer_compute = VK_NULL_HANDLE;

	VkFence fence_compute = VK_NULL_HANDLE;

	// Graphics
	std::array <VkDescriptorSetLayout, NUM_SETS_GRAPHICS> descriptor_set_layouts_graphics = { VK_NULL_HANDLE, VK_NULL_HANDLE };
	VkPipelineLayout pipeline_layout_graphics = VK_NULL_HANDLE;
	VkPipeline pipeline_graphics = VK_NULL_HANDLE;

	VkDescriptorPool descriptor_pool_graphics = VK_NULL_HANDLE;
	vulkan_descriptor_set desc_set_0_graphics_output; // for rendering output image, buffer_graphics_camera + buffer_graphics_model_output
	vulkan_descriptor_set desc_set_1_graphics_output; // for rendering output image, texture_compute_output

	vulkan_buffer buffer_graphics_camera;
	vulkan_buffer buffer_graphics_model_output; // for when rendering the output texture
	vulkan_buffer buffer_density_a;				// For storing density to put into the previous image
	vulkan_buffer buffer_density_b;				// For storing density to put into the rendered image

	VkCommandPool command_pool_graphics = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer_graphics = VK_NULL_HANDLE;

	vulkan_mesh mesh_sprite;

	VkSemaphore swapchain_image_available_semaphore = VK_NULL_HANDLE;
	VkFence fence_submit_graphics = VK_NULL_HANDLE;

	// ImGui
	VkDescriptorPool imgui_descriptor_pool;

	static constexpr float FluidDensity = 1.f;
	static constexpr float CellSize = 1.f;
	// Could turn this into ImGui Editable
	void EmitSmoke(int x, int y, float density, float temp)
	{
		SmokeState* Smoke = &SmokeStates[SmokeIndex];

		int i = GetCellIndex(x, y);
		Smoke->Density[i] += density;
		Smoke->Temperature[i] += temp;

		// TODO: Temp testing values
		Smoke->VelocityY[GetVelocityYIndex(x, y)] = -10.0f;
		Smoke->VelocityY[GetVelocityYIndex(x, y + 1)] = -10.0f;
	}

	int GetCellIndex(int x, int y)
	{
		return x + (y * GridSizeX);
	}
	int GetVelocityXIndex(int x, int y)
	{
		return x + y * (GridSizeX + 1);
	}
	int GetVelocityYIndex(int x, int y)
	{
		return x + y * GridSizeX;
	}

	bool IsSolid(int x, int y)
	{
		return x <= 0 || x >= GridSizeX - 1 ||
			y <= 0 || y >= GridSizeY - 1;
	}

	void PreparePressureSolver(SmokeState* read, float dTime)
	{
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				int flowTop = !IsSolid(x, y + 1);
				int flowLeft = !IsSolid(x - 1, y);
				int flowRight = !IsSolid(x + 1, y);
				int flowBot = !IsSolid(x, y - 1);

				int edgeCount = flowTop + flowLeft + flowRight + flowBot;
				bool isSolid = IsSolid(x, y);

				float velTop = read->VelocityY[GetVelocityYIndex(x, y + 1)];
				float velLeft = read->VelocityX[GetVelocityXIndex(x, y)];
				float velRight = read->VelocityX[GetVelocityXIndex(x + 1, y)];
				float velBot = read->VelocityY[GetVelocityYIndex(x, y)];

				float velTerm = (velRight - velLeft + velTop - velBot) / dTime;

				PressureSolveData[GetCellIndex(x, y)] =
				{
					.flowLeft = (float)flowLeft,
					.flowRight = (float)flowRight,
					.flowTop = (float)flowTop,
					.flowBottom = (float)flowBot,
					.flowEdgeCount = edgeCount,
					.isSolid = isSolid,
					.velocityTerm = velTerm
				};
			}
		}
	}
	void PressureSolve(SmokeState* read)
	{
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				float newPressure;
				PressureSolver info = PressureSolveData[GetCellIndex(x, y)];

				if (info.isSolid || info.flowEdgeCount == 0) newPressure = 0;
				else
				{
					float pressureTop = read->Pressure[GetCellIndex(x, (std::min)(y + 1, GridSizeY - 1))] * info.flowTop;
					float pressureLeft = read->Pressure[GetCellIndex((std::max)(x - 1, 0), y)] * info.flowLeft;
					float pressureRight = read->Pressure[GetCellIndex((std::min)(x + 1, GridSizeX - 1), y)] * info.flowRight;
					float pressureBottom = read->Pressure[GetCellIndex(x, (std::max)(y - 1, 0))] * info.flowBottom;

					float pressureSum = pressureRight + pressureLeft + pressureTop + pressureBottom;
					float CellSize = 1.f;

					newPressure = (pressureSum - FluidDensity * CellSize * info.velocityTerm) / (float)info.flowEdgeCount;
				}

				float oldPressure = read->Pressure[GetCellIndex(x, y)];
				float SOR = 1.f;
				read->Pressure[GetCellIndex(x, y)] = oldPressure + (newPressure - oldPressure) * SOR;
			}
		}
	}
	void RunPressureSolver(SmokeState* read, float dTime)
	{
		int numIts = 80;
		PreparePressureSolver(read, dTime);
		for (int i = 0; i < numIts; i++)
		{
			PressureSolve(read);
		}
	}

	void UpdateVelocities(SmokeState* state, float dTime)
	{
		float K = dTime / (FluidDensity * CellSize);

		// Horizontal velocities
		for (int x = 0; x < GridSizeX + 1; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				if (IsSolid(x - 1, y) || IsSolid(x, y))
					continue;

				float pressureRight =
					state->Pressure[GetCellIndex(x, y)];

				float pressureLeft =
					state->Pressure[GetCellIndex(x - 1, y)];

				state->VelocityX[GetVelocityXIndex(x, y)]
					-= K * (pressureRight - pressureLeft);
			}
		}

		// Vertical velocities
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY + 1; y++)
			{
				if (IsSolid(x, y - 1) || IsSolid(x, y))
					continue;

				float pressureTop =
					state->Pressure[GetCellIndex(x, y)];

				float pressureBottom =
					state->Pressure[GetCellIndex(x, y - 1)];

				state->VelocityY[GetVelocityYIndex(x, y)]
					-= K * (pressureTop - pressureBottom);
			}
		}
	}

	float SampleBilinear(const float* data, int width, int height, float x, float y)
	{
		x = std::clamp(x, 0.0f, (float)(width - 1));
		y = std::clamp(y, 0.0f, (float)(height - 1));

		int x0 = (int)std::floor(x);
		int y0 = (int)std::floor(y);

		int x1 = (std::min)(x0 + 1, width - 1);
		int y1 = (std::min)(y0 + 1, height - 1);

		float tx = x - x0;
		float ty = y - y0;

		float bottom = std::lerp(
			data[x0 + y0 * width],
			data[x1 + y0 * width],
			tx
		);

		float top = std::lerp(
			data[x0 + y1 * width],
			data[x1 + y1 * width],
			tx
		);

		return std::lerp(bottom, top, ty);
	}
	glm::vec2 GetVelocityAtGridPos(SmokeState* state, glm::vec2 pos)
	{
		float velX = SampleBilinear(
			state->VelocityX,
			GridSizeX + 1,
			GridSizeY,
			pos.x,
			pos.y - 0.5f
		);

		float velY = SampleBilinear(
			state->VelocityY,
			GridSizeX,
			GridSizeY + 1,
			pos.x - 0.5f,
			pos.y
		);

		return glm::vec2(velX, velY);
	}

	void AdvectDye(SmokeState* state, float dTime)
	{
		float CellSize = 1.0f;

		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				int i = GetCellIndex(x, y);

				if (IsSolid(x, y))
				{
					TempDensity[i] = 0.0f;
					continue;
				}

				glm::vec2 pos(
					x + 0.5f,
					y + 0.5f
				);

				glm::vec2 vel = GetVelocityAtGridPos(state, pos);

				glm::vec2 posPrev =
					pos - vel * (dTime / CellSize);

				TempDensity[i] = SampleBilinear(
					state->Density,
					GridSizeX,
					GridSizeY,
					posPrev.x - 0.5f,
					posPrev.y - 0.5f
				);
			}
		}

		// Copy temporary density back
		for (int i = 0; i < GridSizeX * GridSizeY; i++)
		{
			state->Density[i] = TempDensity[i];
		}
	}
	float SampleVelocityX(SmokeState* state, glm::vec2 pos)
	{
		return SampleBilinear(
			state->VelocityX,
			GridSizeX + 1,
			GridSizeY,
			pos.x,
			pos.y - 0.5f
		);
	}

	float SampleVelocityY(SmokeState* state, glm::vec2 pos)
	{
		return SampleBilinear(
			state->VelocityY,
			GridSizeX,
			GridSizeY + 1,
			pos.x - 0.5f,
			pos.y
		);
	}
	void AdvectVelocity(SmokeState* state, float dTime)
	{
		float CellSize = 1.0f;

		for (int x = 0; x < GridSizeX + 1; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				int i = GetVelocityXIndex(x, y);

				if (IsSolid(x - 1, y) || IsSolid(x, y))
				{
					TempVelocityX[i] = 0.0f;
					continue;
				}

				glm::vec2 pos(
					(float)x,
					y + 0.5f
				);

				glm::vec2 vel = GetVelocityAtGridPos(state, pos);

				glm::vec2 posPrev =
					pos - vel * (dTime / CellSize);

				TempVelocityX[i] = SampleVelocityX(state, posPrev);
			}
		}

		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY + 1; y++)
			{
				int i = GetVelocityYIndex(x, y);

				if (IsSolid(x, y - 1) || IsSolid(x, y))
				{
					TempVelocityY[i] = 0.0f;
					continue;
				}

				glm::vec2 pos(
					x + 0.5f,
					(float)y
				);

				glm::vec2 vel = GetVelocityAtGridPos(state, pos);

				glm::vec2 posPrev =
					pos - vel * (dTime / CellSize);

				TempVelocityY[i] = SampleVelocityY(state, posPrev);
			}
		}

		for (int i = 0; i < (GridSizeX + 1) * GridSizeY; i++)
		{
			state->VelocityX[i] = TempVelocityX[i];
		}

		for (int i = 0; i < GridSizeX * (GridSizeY + 1); i++)
		{
			state->VelocityY[i] = TempVelocityY[i];
		}
	}
	
	// Smoke Simulation
	// TODO: Move this into the compute shader
	void UpdateSmoke(float dTime)
	{
		if (dTime <= 0)
			return;
		SmokeState* read = &SmokeStates[SmokeIndex];
		SmokeState* write = &SmokeStates[1 - SmokeIndex];

		RunPressureSolver(read, dTime);
		UpdateVelocities(read, dTime);
		AdvectDye(read, dTime);;
		AdvectVelocity(read, dTime);


		//SmokeIndex = 1 - SmokeIndex;
		// Re-copy memory to the GPU. This will not be needed once in the compute shader
		// As they will be modified on the GPU not on the CPU
		if (!map_and_unmap_memory(
			device,
			SmokeIndex == 0 ? buffer_density_a.memory : buffer_density_b.memory,
			[this](void* mem)
			{
				memcpy(mem, SmokeStates[SmokeIndex].Density, sizeof(float) * CellCount);
			}
		))
		{
			assert(false, "Unable to copy new density memory");
			return;
		}
	}

	float TempVelocityX[CellCount + GridSizeX]{};
	float TempVelocityY[CellCount + GridSizeY]{};
	float TempDensity[CellCount]{};
	float TempTemperature[CellCount]{};

	PressureSolver PressureSolveData[CellCount]{};

	SmokeState SmokeStates[2];
	int SmokeIndex = 0;
};