#include <iostream>
#include "Application.h"
Application* Application::s_instance = nullptr;
Application::Application() {
	if (s_instance == nullptr)
		s_inbstance = this;
	m_window = std::make_unique<Window>(640,480,"Skips-Engine");
	m_window->SetCallbackFunction(Application::OnEvent);
}
void Application::Run() {
	while (m_running) {
		glClear(GL_COLOR_BUFFER_BIT);
		m_window->SwapBuffers();
		m_window->PollEvents();
		std::cout << m_running << '\n';
	}
}
void Application::OnEvent(Event& e) {
	EventDispatcher dispatcher;
	dispatcher.Dispatch<WindowCloseEvent>(OnWindowClose);
	dispatcher.Dispatch<WindowResizeEvent>(OnWindowResize);
}
void Application::OnWindowClose(WindowCloseEvent& e) {
	m_running = false;
}
void Application::OnWindowResize(WindowResizeEvent& e) {
	glViewport(0, 0, e.GetWidth(), e.GetHeight());
}