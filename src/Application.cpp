
#include "Log.h"
#include <utility>
#include "Helpers.h"
#include "Application.h"
Application* Application::s_instance = nullptr;
Application::Application() {
	if (s_instance == nullptr)
		s_instance = this;
	m_window = std::make_unique<Window>(640,480,"Skips-Engine");
	m_window->SetCallbackFunction(TO_EVENT_FN(OnEvent));
	LOG_INFO("App Created");
}
void Application::Run() {
	LOG_INFO("App Started");
	while (m_running) {
		glClear(GL_COLOR_BUFFER_BIT);
		m_window->SwapBuffers();
		m_window->PollEvents();
	}
}
void Application::OnEvent(Event& e) {
	LOG_INFO(e.GetName());
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>(TO_EVENT_FN(OnWindowClose));
	dispatcher.Dispatch<WindowResizeEvent>(TO_EVENT_FN(OnWindowResize));
}
bool Application::OnWindowClose(WindowCloseEvent& e) {
	m_running = false;
	return true;
}
bool Application::OnWindowResize(WindowResizeEvent& e) {
	glViewport(0, 0, e.GetWidth(), e.GetHeight());
	return true;
}