#pragma once
#include <memory>
#include "input/Input.h"
#include "events/Event.h"
#include "events/WindowEvents.h"
#include "core/Window.h"
#include "render/Renderer.h"
class Application {
public:
	Application();
	~Application() = default;

	Application& Get() const { return *s_instance; }
	void OnEvent(Event& e);
	bool OnWindowClose(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);
	void Run();

private:
	static Application* s_instance;

	std::unique_ptr<Window> m_window;
	std::unique_ptr<Renderer> m_renderer;
	Input m_input;
	bool m_running = true;
};