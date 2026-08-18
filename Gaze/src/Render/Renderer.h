#pragma once
#include "Render/VertexArray.h"
#include "Render/Shader.h"
#include "Render/Mesh.h"
#include "Scene/EntityRegistry.h"
namespace Gaze {
	struct CameraUniformPass {
		glm::mat4 proj;
		glm::mat4 view;
		CameraUniformPass(glm::mat4 p, glm::mat4 v) :proj(p), view(v) {}
		CameraUniformPass() = default;
	};
	class Renderer {
	public:
		static void Init();
		static void OnWindowResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		static void BeginFrame();
		static void Draw(const Transform& transform, const Mesh& mesh, Shader& shader);
		static void SetUniformBuffer(const CameraUniformPass& pass);
		static void SetWireFrameMode(bool enabled);
	private:
		inline static std::unique_ptr<UniformBuffer> s_uniformBuffer = nullptr;
	};
}