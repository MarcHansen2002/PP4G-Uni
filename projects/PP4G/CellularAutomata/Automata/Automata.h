#pragma once

#include <algorithm>
#include "imgui.h"

#include <vulkan/vulkan.h>
#include "../../vulkan_resources.h"
#include <nmmintrin.h>

constexpr static int GridSizeX = 256;
constexpr static int GridSizeY = 256;
constexpr static int CellCount = GridSizeX * GridSizeY;

enum GameTypes
{
	E_GameOfLife,
};

class Automata
{
public:
	Automata() {};
	vulkan_buffer CellBuffer;
	float Cells[CellCount]{};
	virtual void UpdateOnceScalar() {};
	virtual void UpdateOnceSIMD() {};
	virtual void Inspector() {};

	void UpdateOnce(bool SIMD)
	{
		SIMD ? UpdateOnceSIMD() : UpdateOnceScalar();
	}

	bool Alive(int x, int y)
	{
		if (x >= GridSizeX || x < 0) return false;
		if (y >= GridSizeY || y < 0) return false;
		int i = GetCellIndex(x, y);
		return (Cells[i] > 0);
	}
	int GetCellIndex(int x, int y)
	{
		return x + (y * GridSizeX);
	}
	__m128 GetCellIndexSIMD(__m128 x, __m128 y)
	{
		__m128 xSize = _mm_set1_ps(GridSizeX);
		y = _mm_mul_ps(y, xSize);
		return _mm_add_ps(x, y);
	}
	vec2 GetCellCoord(int index)
	{
		return vec2(index % GridSizeX, index / GridSizeY);
	}
	void CreateBuffers(VkPhysicalDevice physDev, VkDevice dev)
	{
		if (!create_vulkan_buffer(physDev, dev,
			sizeof(float) * CellCount,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			CellBuffer))
		{
			assert(false);
			return;
		}
	}
	void MapBuffer(VkDevice dev)
	{
		if (!map_and_unmap_memory(
			dev,
			CellBuffer.memory, [this](void* mem)
			{
				memcpy(mem, Cells, sizeof(float) * CellCount);
			}
		))
		{
			assert(false);
			return;
		}
	}
	void Terminate(VkDevice dev)
	{
		release_vulkan_buffer(dev, CellBuffer);
	}
private:
};