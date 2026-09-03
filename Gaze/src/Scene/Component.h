#pragma once
#include "Render/Mesh.h"
#include "Assets/AssetManager.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
namespace Gaze {
	enum class ComponentType {
		Transform,MeshRenderer,HierarchyMember
	};
	inline static std::string ComponentTypeToString(ComponentType type) {
		switch (type) {
			case ComponentType::Transform:
				return "Transform";
			case ComponentType::MeshRenderer:
				return "MeshRenderer";
			case ComponentType::HierarchyMember:
				return "HierarchyMember";
		}
	}
	//modify only through Entity (Entity.SetTransform .. etc ) , modyfying directly doesn't update children transforms : Entity.GetComponent<Transform>() this doesn't mark dirty
	class Transform {
	public:
		Transform(const glm::vec3 pos, const glm::vec3 rot, const glm::vec3 scl) :translation(pos), rotation(glm::quat(rot)), scale(scl) { worldMatrix = glm::mat4(1.0f); }
		Transform() {
			translation = glm::vec3(0.0f);
			rotation = glm::quat(glm::vec3(0.0f));
			scale = glm::vec3(1.0f);
			worldMatrix = glm::mat4(1.0f);
		}

		void Rotate(glm::vec3 eulerAngles) {
			glm::quat rotationquat(glm::radians(eulerAngles));
			rotation *= rotationquat;
		}
		void SetRotation(glm::vec3 eulerAngles) {
			rotation = glm::quat(glm::radians(eulerAngles));
		}
		glm::vec3 GetRotationEuler() const { return glm::degrees(glm::eulerAngles(rotation)); }

		void Rotate(glm::quat quat) {
			rotation *= quat;
		}
		void SetRotation(glm::quat quat) {
			rotation = quat;
		}
		glm::quat GetRotationQuat() const { return rotation; }

		void SetScale(glm::vec3 newscale) {
			scale = newscale;
		}
		void Scale(glm::vec3 newscale) {
			scale += newscale;
		}
		glm::vec3 GetScale() const { return scale; }

		void SetPosition(glm::vec3 pos) {
			translation = pos;
		}
		void Translate(glm::vec3 pos) {
			translation += pos;
		}
		glm::vec3 GetPosition() const { return translation; }

		void SetFromMatrix(glm::mat4 matrix) {
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(matrix, scale, rotation, translation, skew, perspective);
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
			return worldMatrix;
		}
		void ComputeMatrix(const glm::mat4& parentWorldMatrix = glm::mat4(1.0f)) {
			glm::mat4 model(1.0f);
			model = glm::translate(model, translation);

			model *= glm::mat4_cast(rotation);

			model = glm::scale(model, scale);
			
			worldMatrix = parentWorldMatrix * model;
		}
	private:
		glm::vec3 translation; // local
		glm::quat rotation; // local 
		glm::vec3 scale; // local

		glm::mat4 worldMatrix;
	};
	struct MeshRenderer {
		AssetHandle<Mesh> mesh;
		AssetHandle<Material> material; // vector<AssetHandle<Material>> materials (1 submesh - > 1 material)
		MeshRenderer() {
			mesh.id = ReservedUUID::CUBE;
			material.id = ReservedUUID::DEFAULTMATERIAL;
			mesh.asset = AssetManager::Get<Mesh>(mesh.id);
			material.asset = AssetManager::Get<Material>(material.id);
		}
		
		void LoadMesh(const UUID& id) {
			mesh.id = id;
			mesh.asset = AssetManager::Get<Mesh>(id);
		}

	};
	struct HierarchyMember {
		uint32_t parent{};
		std::vector<uint32_t> children{};
		HierarchyMember() :parent(0){}
	};
}