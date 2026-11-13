#pragma once
#include "../Automata.h"
#include <vector>

class GameOfLife : public Automata
{
public:
	void UpdateOnceScalar() override
	{
		float* newCells = new float[CellCount] {};
		
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				int i = GetCellIndex(x, y);
				int aliveCount = 0;
				for (int ax = -1; ax < 2; ax++)
				{
					for (int ay = -1; ay < 2; ay++)
					{
						if (ax == 0 && ay == 0) continue;
						aliveCount += Alive(x + ax, y + ay) ? 1 : 0;
					}
				}
		
				if (Alive(x, y))
				{
					bool result = false;
					for (int x : LiveRule)
					{
						if (x == aliveCount)
						{
							result = true;
							break;
						}
					}
					newCells[i] = result;
				}
				else
				{
					bool result = false;
					for (int x : BirthRule)
					{
						if (x == aliveCount)
						{
							result = true;
							break;
						}
					}
					newCells[i] = result;
				}
			}
		}
		memcpy(Cells, newCells, sizeof(float) * CellCount);
		delete[] newCells;
	}

	void UpdateOnceSIMD() override
	{
		__m128 zero = _mm_set1_ps(0);
		__m128 one = _mm_set1_ps(1);
		__m128 two = _mm_set1_ps(2);
		__m128 three = _mm_set1_ps(3);

		float* newCells = new float[CellCount] {};	// Array of new cells so we don't overwrite the old ones before reading
		__m128* check = (__m128*)Cells;				// Pointer to start of original array
		for (int i = 0; i < CellCount; i+=4)
		{
			__m128 aliveCount = _mm_set_ps1(0);		 // Start alive count at 0
			vec2 startCoord = GetCellCoord(i);		 // Coordinate X,Y of first checked cell

			// Load all cordinates of each checked cell
			vec2 a = GetCellCoord(i);
			vec2 b = GetCellCoord(i+1);
			vec2 c = GetCellCoord(i+2);
			vec2 d = GetCellCoord(i+3);

			__m128 cellX = _mm_set_ps(d.x, c.x, b.x, a.x);
			__m128 cellY = _mm_set_ps(d.y, c.y, b.y, a.y);

			for (int x = -1; x < 2; x++)
			{
				for (int y = -1; y < 2; y++)
				{
					if (x == 0 && y == 0) continue;

					// Check is valid cell for checking if a neighbour is alive
					__m128 xCheck = _mm_add_ps(cellX, _mm_set1_ps(x));
					__m128 yCheck = _mm_add_ps(cellY, _mm_set1_ps(y));

					__m128 validCellX = _mm_and_ps(_mm_cmplt_ps(xCheck, _mm_set1_ps(GridSizeX)), _mm_cmpge_ps(xCheck, zero));
					__m128 validCellY = _mm_and_ps(_mm_cmplt_ps(yCheck, _mm_set1_ps(GridSizeY)), _mm_cmpge_ps(yCheck, zero));
					__m128 validCell = _mm_and_ps(validCellX, validCellY);

					// Load values of neighbours, Row major so Y changes should be accounted for
					int neighbourIndex = GetCellIndex(startCoord.x + x, startCoord.y + y);
					__m128 neighbour = _mm_load_ps(Cells + neighbourIndex);
					// Set the values for invalid cells to 0 whilst still using value from valid cells
					neighbour = _mm_and_ps(neighbour, validCell);
					// Add any living neighbours to the count of living neighbours
					aliveCount = _mm_add_ps(aliveCount, neighbour);
				}
			}

			// Set new state
			__m128 alive = _mm_cmpeq_ps(*check, one);				// 1 if alive, 0 if dead
			__m128 twoAlive = _mm_cmpeq_ps(aliveCount, two);		// 1 if 2 neighbours
			__m128 threeAlive = _mm_cmpeq_ps(aliveCount, three);	// 1 if 3 neighbours

			__m128 survives = _mm_and_ps(alive, _mm_or_ps(twoAlive, threeAlive));	// 1 if the cell should survive
			__m128 born = _mm_andnot_ps(alive, threeAlive);	// 1 if the cell should be born

			__m128 result = _mm_and_ps(_mm_or_ps(survives, born), one);
			_mm_store_ps(newCells + i, result);
			check++; // Go to next 4 values
		}
		
		memcpy(Cells, newCells, sizeof(float) * CellCount);
		delete[] newCells;
	}

	void Inspector() override
	{
		ImGui::Text("Game Of Life");
		ImGui::PushID("Birth");
		for (int i = 0; i < BirthRule.size(); i++)
		{
			ImGui::PushID(i);
			ImGui::SliderInt("Birth", &BirthRule[i], 0, 8);
			ImGui::PopID();
		}
		if (ImGui::Button("+")) { BirthRule.push_back(0); }
		ImGui::SameLine();
		if (ImGui::Button("-")) { BirthRule.pop_back(); }
		ImGui::PopID();
		ImGui::PushID("Live");
		for (int i = 0; i < LiveRule.size(); i++)
		{
			ImGui::PushID(i);
			ImGui::SliderInt("Live", &LiveRule[i], 0, 8);
			ImGui::PopID();
		}
		if (ImGui::Button("+")) { LiveRule.push_back(0); }
		ImGui::SameLine();
		if (ImGui::Button("-")) { LiveRule.pop_back(); }
		ImGui::PopID();
	}

private:
	std::vector<int> BirthRule = {3};
	std::vector<int> LiveRule = { 2, 3 };
};