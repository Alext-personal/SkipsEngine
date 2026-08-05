
#include "core/Log.h"
#include "core/Helpers.h"
#include "core/Application.h"
Application* Application::s_instance = nullptr;
Application::Application() : m_window(std::make_unique<Window>(640, 480, "Skips-Engine")),m_renderer(std::make_unique<Renderer>())
{
	if (s_instance != nullptr)
	{
		Log::ERROR("MULTIPLE APPLICATIONS, SHUTTING DOWN");
	}
	s_instance = this;
	m_window->SetCallbackFunction(TO_EVENT_FN(OnEvent));
	Log::INFO("App Created");
}
void Application::Run() {
	Log::INFO("App Started");
	while (m_running) {
		glClear(GL_COLOR_BUFFER_BIT);
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
	glViewport(0, 0, e.GetWidth(), e.GetHeight());
	return true;
}