#include "Automata.h"

void Automata::Inspector()
{
	ImGui::Text("Frame Index: %i", timer.GetIndex());
	float avg = timer.GetAverage();
	ImGui::Text("Average Execution Time: %.2f (ms)", avg);
	ImGui::Text("As FPS: %.2f", 1000.f / avg);
	if (ImGui::Button("Clear Recordings"))
	{
		timer.Clear();
	}
	ImGui::NewLine();
}