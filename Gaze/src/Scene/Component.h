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
		glm::quat rotation{ glm::vec3(0.0f)};
		glm::vec3 scale{ 1.0f };

		void Rotate(glm::vec3 eulerAngles) {
			glm::quat rotationquat(glm::radians(eulerAngles));
			rotation *= rotationquat;
		}
		void SetRotation(glm::vec3 eulerAngles) {
			rotation = glm::quat(glm::radians(eulerAngles));
		}
		void Rotate(glm::quat quat) {
			rotation *= quat;
		}
		void SetRotation(glm::quat quat) {
			rotation = quat;
		}
		glm::vec3 GetForward() const {
			return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
		}
		glm::vec3 GetUp() const {
			return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
		}
		glm::vec3 GetRight() const {
			return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
		}
		glm::mat4 GetMatrix() const {
			glm::mat4 model(1.0f);
			model = glm::translate(model, translation);

			model *= glm::mat4_cast(rotation);

			model = glm::scale(model, scale);

			return model;
		}
	};
	struct MeshRenderer {
		AssetHandle<Mesh> mesh;
		AssetHandle<Shader> shader;
		MeshRenderer() {
			mesh.id = ReservedUUID::CUBE;
			shader.id = ReservedUUID::DEFAULTSHADER;
			mesh.asset = AssetManager::Get<Mesh>(mesh.id);
			shader.asset = AssetManager::Get<Shader>(shader.id);
		}
		
		void LoadMesh(const UUID& id) {
			mesh.id = id;
			mesh.asset = AssetManager::Get<Mesh>(id);
		}

	};
}