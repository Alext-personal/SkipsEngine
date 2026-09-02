#include "pch.h"
#include "Imgui/ImguiLayer.h"
#include "Core/Application.h"
namespace Gaze {
	ImguiLayer::ImguiLayer() :Layer("ImGUI") {}
	ImguiLayer::~ImguiLayer() = default;
	void ImguiLayer::OnUpdate(float dt)
	{
		testTransform.SetPosition({ pos[0],pos[1],pos[2] });
		testTransform.SetScale({ scl[0],scl[1],scl[2] });
		testTransform.SetRotation({ rot[0],rot[1],rot[2] }); // testing
	}
	void ImguiLayer::OnEvent(Event& e)
	{
	}
	void ImguiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(Application::Get()->GetWindow().GetNativeWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 460");
	}
	void ImguiLayer::OnDetach() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	void ImguiLayer::Begin() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	void ImguiLayer::End() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	void ImguiLayer::OnImguiRender() {
		ImGui::Begin("Cube Transform");
		ImGui::Text("Default Cube Transform");
		ImGui::DragFloat3("Position", pos ,0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_ColorMarkers);
		ImGui::DragFloat3("Rotation", rot,0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_ColorMarkers);
		ImGui::DragFloat3("Scale", scl ,0.005f, -FLT_MAX, +FLT_MAX, "%.3f", ImGuiSliderFlags_ColorMarkers);
		ImGui::End();
	}

}