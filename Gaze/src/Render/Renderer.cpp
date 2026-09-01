#include "pch.h"
#include "Render/Renderer.h"
#include "Render/EditorCamera.h"
#include "Render/VertexArray.h"
namespace Gaze {
	void Renderer::Init() {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			ENGINE_ASSERT(0, "Error at glad initialisation");
		s_uniformBuffer = std::make_unique<UniformBuffer>(sizeof(CameraUniformPass), 0);
	}
	void Renderer::BeginFrame() {
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	void Renderer::Draw(const Transform& transform, const Mesh& mesh, Material& material) {
		mesh.GetVAO().Bind();
		material.Bind();
		material.shader.asset->SetUniformMatrix4("modelMatrix", transform.GetMatrix());
		if (!mesh.HasEBO()) {
			glDrawArrays(GL_TRIANGLES, 0, mesh.GetVertexCount());
		}
		else {
			if (!mesh.GetSubMeshes().empty())
				for (const auto& submesh : mesh.GetSubMeshes()) {
					glDrawElements(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, (void*)(submesh.indexOffset * sizeof(uint32_t)));
				}
			else
				glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
		}

	}
	void Renderer::SetUniformBuffer(const CameraUniformPass& pass) {
		s_uniformBuffer->SetData(&pass, sizeof(pass), 0);
	}
	void Renderer::SetWireFrameMode(bool enabled) {
		if (enabled)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	void Renderer::OnWindowResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
		glViewport(x, y, width, height);
	}
}