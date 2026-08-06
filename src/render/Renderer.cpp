#include "render/Renderer.h"
#include "core/Log.h"
#include <GLFW/glfw3.h>


void Renderer::Init() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		Log::ERROR("Error at glad initialisation");
}
void Renderer::PreDraw() {
	glDisable(GL_DEPTH_TEST); 
	glDisable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Renderer::Draw(const VertexArray& vao, const ShaderProgram& shader){
	vao.Bind();
	shader.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
	glViewport(0, 0, width, height);
}