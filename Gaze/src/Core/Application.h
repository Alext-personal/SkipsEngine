#pragma once
#include "Core/Layer.h"
#include "Core/LayerStack.h"
#include "Core/Window.h"
#include "Scene/Scene.h"
#include "Input/Input.h"
#include "Events/Event.h"
#include "Events/WindowEvents.h"
#include "Imgui/ImguiLayer.h"
#include <memory>
namespace Gaze {
	class Application {
	public:
		Application();
		~Application() = default;

		static Application* Get() { return s_instance; }
		Window& GetWindow() const { return *m_window; }
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		void OnEvent(Event& e);
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void Run();

	private:
		inline static Application* s_instance = nullptr;
		std::unique_ptr<Window> m_window;
		Input m_input;
		LayerStack m_layerStack;
		ImguiLayer* m_imguiLayer;
		bool m_running = true;
		Scene m_activeScene{}; //TEMPORARY !!!!!!!!!
	};
}