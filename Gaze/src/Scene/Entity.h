#pragma once
#include "Scene/Scene.h"
#include "Core/UUID.h"
namespace Gaze {
	class Entity { // wrapper over ecs entity for ease of use
	public:
		Entity(Scene& scene,const UUID& persistentID = ReservedUUID::NONE) {
			m_scene = &scene;
			m_entityID = m_scene->GetRegistry().CreateEntity(persistentID);
			m_persistentID = m_scene->GetRegistry().GetUUID(m_entityID);
			if (persistentID == ReservedUUID::NONE) {
				AddComponent<Transform>(); // every entity has a transform so far
				AddComponent<HierarchyMember>(); // every entity is apart of the hierarchy
			}
			//it gets loaded by scene if it has an uuid
		}
		Entity(Scene& scene,uint32_t entityID) { // get wrapper around entity
			//maybe check if exists, if not return null entity ?
			m_scene = &scene;
			m_entityID = entityID;
			m_persistentID = m_scene->GetRegistry().GetUUID(entityID);
		}
		~Entity() {
			m_scene = nullptr;
		}
		void Destroy() {
			GetParent().RemoveChild(*this);
			DestroyInternal();
		}
		template <typename T>
		T& AddComponent() {
			return m_scene->GetRegistry().AddComponent<T>(m_entityID);
		}
		template <typename T>
		void RemoveComponent() {
			m_scene->GetRegistry().RemoveComponent<T>(m_entityID);
		}
		template <typename T>
		T& GetComponent() {
			return m_scene->GetRegistry().GetComponent<T>(m_entityID);
		}
		template <typename T>
		bool HasComponent() {
			return m_scene->GetRegistry().HasComponent<T>(m_entityID);
		}

		UUID& GetUUID() { return m_persistentID; }
		Entity GetParent();
		std::vector<Entity> GetChildren();
		void SetParent(Entity& parent);
		void AddChild(Entity& child);
		void RemoveChild(Entity& child);

		void Rotate(glm::vec3 eulerAngles);
		void SetRotation(glm::vec3 eulerAngles);
		glm::vec3 GetRotationEuler();

		void Rotate(glm::quat quat);
		void SetRotation(glm::quat quat);
		glm::quat GetRotationQuat();

		void SetScale(glm::vec3 newscale);
		void Scale(glm::vec3 newscale);
		glm::vec3 GetScale();

		void SetPosition(glm::vec3 pos);
		void Translate(glm::vec3 pos);
		glm::vec3 GetPosition();

		void SetTransform(const Transform& t);
		void SetMesh(const UUID& meshID);
		void SetMaterialSlot(uint32_t slot, const UUID& materialID);

	private:
		uint32_t m_entityID{};
		UUID m_persistentID;
		Scene* m_scene = nullptr;
		void DestroyInternal() {
			m_scene->GetTransformSystem()->UnMarkDirty(m_entityID);
			for (Entity& child : GetChildren()) {
				child.DestroyInternal();
			}
			m_scene->GetRegistry().DeleteEntity(m_entityID);
		}
	};
}