
#include "core/Log.h"
#include "core/Helpers.h"
#include "core/Application.h"
#include "render/Renderer.h"
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
	std::vector<GLfloat> data{
			-0.8f,-0.8f,0.0f,
			0.8f,-0.8f,0.0f,
			0.0f,0.8f,0.0f
	};
	VertexArray vao;
	VertexLayout layout;
	Shader vertex(GL_VERTEX_SHADER, "assets/shaders/Vertex.glsl");
	Shader fragment(GL_FRAGMENT_SHADER, "assets/shaders/Fragment.glsl");
	ShaderProgram shader(vertex, fragment);
	layout.Add(AttributeDataType::Float3);
	VertexBuffer vbo(data.data(), data.size() * sizeof(GLfloat));
	vao.AddVertexBuffer(vbo, layout);
	while (m_running) {

		Renderer::PreDraw();
		
		Renderer::Draw(vao, shader);
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