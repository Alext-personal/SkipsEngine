#pragma once
#include <memory>
#include "Input/Input.h"
#include "Events/Event.h"
#include "Events/WindowEvents.h"
#include "Core/Window.h"
#include "Scene/Scene.h"
namespace Gaze {
	class Application {
	public:
		Application();
		~Application() = default;

		static Application& Get() { return *s_instance; }
		void OnEvent(Event& e);
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		void Run();

	private:
		static Application* s_instance;
		std::unique_ptr<Window> m_window;
		Input m_input;
		bool m_running = true;
		Scene m_activeScene{}; //TEMPORARY !!!!!!!!!
	};
}