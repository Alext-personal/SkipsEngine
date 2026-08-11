#include "Assets/ModelLoader.h"
#include "Core/Log.h"
#include "Core/Helpers.h"
#include "Core/Application.h"
#include "Render/Renderer.h"
#include "Render/Primitives/Primitives.h"
#include <glm/glm.hpp>// TEMP
#include <glm/gtc/matrix_transform.hpp> // TEMP
#include "Ecs/EntityRegistry.h" //temp
Application* Application::s_instance = nullptr;
Application::Application() : m_window(std::make_unique<Window>(640, 480, "Skips-Engine"))
{
	if (s_instance != nullptr)
	{
		LOG_ERROR("MULTIPLE APPLICATIONS, SHUTTING DOWN");
	}
	s_instance = this;
	m_window->SetCallbackFunction(TO_EVENT_FN(OnEvent));
	Renderer::Init();
	Log::INFO("App Created");
}
void Application::Run() {
	Log::INFO("App Started");
	EntityRegistry ECS;
	uint32_t entity = ECS.CreateEntity();
	Transform& transform = ECS.AddComponent<Transform>(entity);
	transform.scale = { .25f,.25f,.25f };
	MeshRenderer& mrenderer = ECS.AddComponent<MeshRenderer>(entity);
	//mrenderer.LoadMesh("assets/models/Car.obj");
	mrenderer.LoadMesh(PrimitiveType::Cube);
	bool wireframe = true; //testing
	float lastTime = 0.0f;
	while (m_running) {
		Renderer::BeginFrame();
		float currentTime = glfwGetTime();
		float timeStep = currentTime - lastTime;
		lastTime = currentTime;
		if (Input::IsKeyTapped(KeyCode::Escape)) {
			Renderer::SetWireFrameMode(wireframe);
			wireframe = !wireframe;
			LOG_WARNING("wireframe set: ", wireframe);
		}
		transform.rotation.y += 50.0f * timeStep;




		Renderer::DrawScene(ECS); //m_activescene
		Input::OnFrameEnd();
		m_window->SwapBuffers();
		m_window->PollEvents();
	}
}
void Application::OnEvent(Event& e) {
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>(TO_EVENT_FN(OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(TO_EVENT_FN(OnWindowResize));
	m_input.OnEvent(e);
}
bool Application::OnWindowClose(WindowCloseEvent& e) {
	m_running = false;
	return true;
}
bool Application::OnWindowResize(WindowResizeEvent& e) {
	Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
	return true;
}