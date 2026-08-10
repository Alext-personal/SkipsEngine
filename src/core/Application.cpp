#include "Assets/ModelLoader.h"
#include "Core/Log.h"
#include "Core/Helpers.h"
#include "Core/Application.h"
#include "Render/Renderer.h"
#include "Render/Primitives/Primitives.h"
#include <glm/glm.hpp>// TEMP
#include <glm/gtc/matrix_transform.hpp> // TEMP
#include "Ecs/EntityRegistry.h" //temp
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
	Mesh renderedMesh(ModelLoader::LoadModel("assets/models/Car.obj"));
	EntityRegistry ECS;
	uint32_t entity = ECS.CreateEntity();
	Transform& transform = ECS.AddComponent<Transform>(entity);
	transform.Scale = glm::vec3(.25f, .25f, .25f);
	transform.Rotation = glm::vec3(45.0f, 0.0f, 0.0f);
	glm::mat4 model(1.0f);
	model = glm::scale(model, transform.Scale);
	bool wireframe = true; //testing
	float lastTime = 0.0f;
	while (m_running) {
		float currentTime = glfwGetTime();
		float timeStep = currentTime - lastTime;
		lastTime = currentTime;
		Renderer::PreDraw();
		if (Input::IsKeyTapped(KeyCode::Escape)) {
			Renderer::SetWireFrameMode(wireframe);
			wireframe = !wireframe;
			LOG_WARNING("wireframe set: ", wireframe);
		}
		transform.Rotation.x = 1.0f * timeStep;
		model = glm::rotate(model, transform.Rotation.x, glm::vec3(0, 1, 0));
		shader.SetUniformMatrix4("modelMatrix", model); //testing
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