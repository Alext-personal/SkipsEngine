#include "Render/Renderer.h"
#include "Core/Log.h"
#include <GLFW/glfw3.h>
void Renderer::Init() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		LOG_ERROR("Error at glad initialisation");
}
void Renderer::PreDraw() {
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Renderer::Draw(const Mesh& mesh, const ShaderProgram& shader){
	mesh.GetVAO().Bind();
	shader.Bind();
	if (!mesh.HasEBO()) {
		glDrawArrays(GL_TRIANGLES, 0, mesh.GetVertexCount());
	}
	else {
		if(!mesh.GetSubMeshes().empty())
			for (const auto& submesh : mesh.GetSubMeshes()) {
			glDrawElements(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, (void*)(submesh.indexOffset * sizeof(uint32_t)));
			}
		else
			glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
	}
		
}
void Renderer::SetWireFrameMode(bool enabled) {
	if (enabled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
	glViewport(0, 0, width, height);
}