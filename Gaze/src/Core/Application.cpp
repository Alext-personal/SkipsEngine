#include "pch.h"
#include "Core/Application.h"
#include "Render/Renderer.h"
#include "Render/EditorCamera.h" //temp
#include "Scene/Entity.h" // temp
#include "Resources/EditorTEMP/AssetManager.h"
#include <GLFW/glfw3.h>
namespace Gaze {
	Application::Application() : m_window(std::make_unique<Window>(1920, 1080, "Skips-Engine")), m_activeScene{}
	{
		ENGINE_ASSERT(s_instance == nullptr,"Duplicate Application Instance");
		s_instance = this;
		m_window->SetCallbackFunction(TO_EVENT_FN(OnEvent));
		m_window->SetVSync(false);
		Renderer::Init();
		m_resourceManager.Initialize();
		m_activeScene.Initialize();
		m_assetManager.InitializeAssetsFolder();
		m_assetManager.LoadAssets();
		m_imguiLayer = new ImguiLayer();
		PushOverlay(m_imguiLayer);
		LOG_INFO("App Created");
	}
	void Application::PushLayer(Layer* layer){
		m_layerStack.PushLayer(layer);
	}
	void Application::PushOverlay(Layer* overlay) {
		m_layerStack.PushOverlay(overlay);
	}
	void Application::Run() {
		LOG_INFO("App Started");
		Entity ent = m_activeScene.Instantiate(11825589953740235702);
		auto start = GetTime();
		auto t1 = GetTime();
		LOG_INFO("Mesh loading took: ${} ", t1 - start);
		bool wireframe = true; //testing
		float lastTime = glfwGetTime();
		double accumulated = 0.0f;
		int frames = 0;
		while (m_running) {
			m_window->PollEvents();
			m_resourceManager.OnFrameStart();
			Renderer::BeginFrame();
			float currentTime = glfwGetTime();
			float timeStep = currentTime - lastTime;
			lastTime = currentTime;
			accumulated += timeStep;
			frames++;
			if (frames == 15000)
			{
				double average = accumulated / frames;

				LOG_INFO("Average Frame Time : ${} ms", average * 1000);
				LOG_INFO("Average Fps : ${}", 1.0 / average);

				accumulated = 0.0f;
				frames = 0;
			}
			#pragma region EDITOR
			if (Input::IsKeyTapped(KeyCode::Escape)) { // to be moved to editor 
				Renderer::SetWireFrameMode(wireframe);
				wireframe = !wireframe;
				LOG_WARNING("wireframe set: ${} ", wireframe);
			}
			if (Input::IsKeyTapped(KeyCode::Tab))
				Application::Get().GetWindow().SwitchCursorMode();
			//ent.SetTransform(m_imguiLayer->testTransform);
			//ent2.Rotate(glm::vec3{ 15,0,0 } *timeStep);
			if(Application::Get().GetWindow().IsCursorDisabled())
				EditorCamera::OnUpdate(timeStep); // temp to be moved to editor app
			m_activeScene.OnUpdate(timeStep); //m_activescene to be moved to EDITOR APP
			#pragma endregion
			for (Layer* layer : m_layerStack) {
				layer->OnUpdate(timeStep);
			}
			m_imguiLayer->Begin();
			for (Layer* layer : m_layerStack) {
				layer->OnImguiRender();
			}
			m_imguiLayer->End();
			m_input.OnFrameEnd();
			m_window->SwapBuffers();
		}
	}
	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(TO_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(TO_EVENT_FN(OnWindowResize));
		m_input.OnEvent(e);
		for (auto it = m_layerStack.end(); it != m_layerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.handled)
				break;
		}
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