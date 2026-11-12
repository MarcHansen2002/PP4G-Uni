#pragma once
#include "../Automata.h"
#include <vector>

class GameOfLife : public Automata
{
public:
	void UpdateOnce() override
	{
		float* newCells = new float[CellCount] {};
		
		for (int x = 0; x < GridSizeX; x++)
		{
			for (int y = 0; y < GridSizeY; y++)
			{
				int i = GetCellIndex(x, y);
				int aliveCount = 0;
				if (Alive(x - 1, y)) aliveCount++;
				if (Alive(x + 1, y)) aliveCount++;
				if (Alive(x - 1, y + 1)) aliveCount++;
				if (Alive(x, y + 1)) aliveCount++;
				if (Alive(x + 1, y + 1)) aliveCount++;
				if (Alive(x - 1, y - 1)) aliveCount++;
				if (Alive(x, y - 1)) aliveCount++;
				if (Alive(x + 1, y - 1)) aliveCount++;
		
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