#pragma once
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Render/Mesh.h"
#include "Assets/AssetManager.h"
namespace Gaze {
	struct Transform {
		glm::vec3 translation{ 0.0f };
		glm::vec3 rotation{ 0.0f };
		glm::vec3 scale{ 1.0f };

		glm::quat GetQuaternion() const {
			return glm::quat(glm::radians(rotation));
		}
		glm::vec3 GetForward() const {
			return GetQuaternion() * glm::vec3(0.0f, 0.0f, -1.0f);
		}
		glm::vec3 GetUp() const {
			return GetQuaternion() * glm::vec3(0.0f, 1.0f, 0.0f);
		}
		glm::vec3 GetRight() const {
			return GetQuaternion() * glm::vec3(1.0f, 0.0f, 0.0f);
		}
		glm::mat4 GetMatrix() const {
			glm::mat4 model(1.0f);
			model = glm::translate(model, translation);

			model *= glm::mat4_cast(GetQuaternion());

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
}