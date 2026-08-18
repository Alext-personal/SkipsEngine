#include "Assets/ModelLoader.h" // temp
#include "Core/Log.h"
#include "Core/Helpers.h"
#include "Core/Application.h"
#include "Render/Renderer.h"
#include "Render/Primitives/Primitives.h" // temp
#include "Render/EditorCamera.h" //temp
#include "Scene/Entity.h" // temp
namespace Gaze {
	Application* Application::s_instance = nullptr;
	Application::Application() : m_window(std::make_unique<Window>(1920, 1080, "Skips-Engine")), m_activeScene{}
	{
		if (s_instance != nullptr)
		{
			ENGINE_ASSERT(0, "MULTIPLE APPLICATIONS, SHUTTING DOWN");
		}
		s_instance = this;
		m_window->SetCallbackFunction(TO_EVENT_FN(OnEvent));
		Renderer::Init();
		LOG_INFO("App Created");
	}
	void Application::Run() {
		LOG_INFO("App Started");
		Entity ent(m_activeScene); // temp
		ent.AddComponent<MeshRenderer>().LoadMesh(PrimitiveType::Cube); // temp
		auto start = GetTime();
		auto t1 = GetTime();
		LOG_INFO("Mesh loading took: ${} ", t1 - start);
		bool wireframe = true; //testing
		float lastTime = 0.0f;
		while (m_running) {
			Renderer::BeginFrame();
			float currentTime = glfwGetTime();
			float timeStep = currentTime - lastTime;
			lastTime = currentTime;
			if (Input::IsKeyTapped(KeyCode::Escape)) { // to be moved to editor 
				Renderer::SetWireFrameMode(wireframe);
				wireframe = !wireframe;
				LOG_WARNING("wireframe set: ${} ", wireframe);
			}
			//transform.rotation.y += 50.0f * timeStep;
			EditorCamera::OnUpdate(timeStep); // temp to be moved to editor app
			m_activeScene.OnRender(); //m_activescene to be moved to EDITOR APP
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
		Renderer::OnWindowResize(0, 0, e.GetWidth(), e.GetHeight());
		EditorCamera::OnWindowResize(e.GetWidth(), e.GetHeight());
		return true;
	}
}