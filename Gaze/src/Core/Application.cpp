#include "Assets/ModelLoader.h"
#include "Core/Log.h"
#include "Core/Helpers.h"
#include "Core/Application.h"
#include "Render/Renderer.h"
#include "Render/Primitives/Primitives.h"
#include "Scene/EntityRegistry.h" //temp
#include "Render/EditorCamera.h" //temp
Application* Application::s_instance = nullptr;
Application::Application() : m_window(std::make_unique<Window>(1920, 1080, "Skips-Engine"))
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
	MeshRenderer& mrenderer = ECS.AddComponent<MeshRenderer>(entity);
	transform.scale = { .25f,.25f,.25f };
	mrenderer.LoadMesh("Gaze/assets/models/Car.obj");

	uint32_t entity2 = ECS.CreateEntity();
	Transform& transform2 = ECS.AddComponent<Transform>(entity2);
	MeshRenderer& mrenderer2 = ECS.AddComponent<MeshRenderer>(entity2);
	transform2.scale = { .5f,.5f,.5f };
	transform2.translation = { 1.0f,1.0f,1.0f };
	mrenderer2.LoadMesh(PrimitiveType::Cube);

	auto start = GetTime();
	auto t1 = GetTime();
	Log::INFO("Mesh loading took : ", t1 - start);
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
		//transform.rotation.y += 50.0f * timeStep;
		EditorCamera::OnUpdate(timeStep); // temp
		Renderer::DrawScene(ECS); //m_activescene
		m_input.OnFrameEnd();
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
	Renderer::OnWindowResize(0,0,e.GetWidth(), e.GetHeight());
	EditorCamera::OnWindowResize(e.GetWidth(),e.GetHeight());
	return true;
}