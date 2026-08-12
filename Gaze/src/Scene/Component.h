#pragma once
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Render/Mesh.h"
#include "Assets/AssetManager.h"
struct Transform {
	glm::vec3 translation{ 0.0f };
	glm::vec3 rotation{ 0.0f };
	glm::vec3 scale{ 1.0f };

	glm::mat4 GetMatrix() {
		glm::mat4 model(1.0f);
		model = glm::translate(model, translation);

		model = glm::rotate(model, glm::radians(rotation.x), { 1, 0, 0 });
		model = glm::rotate(model, glm::radians(rotation.y), { 0, 1, 0 });
		model = glm::rotate(model, glm::radians(rotation.z), { 0, 0, 1 });

		model = glm::scale(model, scale);

		return model;
	}
};
struct MeshRenderer {
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Shader> shader;
	MeshRenderer() {
		mesh = AssetManager::GetMesh(PrimitiveType::Quad);
		shader = AssetManager::GetShader();
	}
	void LoadMesh(const std::filesystem::path& filepath) {
		mesh = AssetManager::GetMesh(filepath);
	}
	void LoadMesh(PrimitiveType type) {
		mesh = AssetManager::GetMesh(type);
	}
	
};