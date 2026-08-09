
#include "Core/Log.h"
#include "Core/Helpers.h"
#include "Core/Application.h"
#include "Render/Renderer.h"
#include "Render/Primitives/Primitives.h"
#include <glm/glm.hpp>// TEMP
#include <glm/gtc/matrix_transform.hpp> // TEMP
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
	Shader vertex(GL_VERTEX_SHADER, "assets/shaders/Vertex.glsl");
	Shader fragment(GL_FRAGMENT_SHADER, "assets/shaders/Fragment.glsl");
	ShaderProgram shader(vertex, fragment);
	Mesh renderedMesh(Primitives::Cube());
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 1.0f, 0)); // testing
	shader.SetUniformMatrix4("modelMatrix", model); //testing
	bool wireframe = true; //testing
	while (m_running) {

		Renderer::PreDraw();
		if (Input::IsKeyTapped(KeyCode::Escape)) {
			Renderer::SetWireFrameMode(wireframe);
			wireframe = !wireframe;
			Log::WARNING("wireframe set: ", wireframe);
		}
		Renderer::Draw(renderedMesh, shader);
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