#pragma once
#include "Scene/Scene.h"
#include "Core/UUID.h"
namespace Gaze {
	class Entity {
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
			m_scene = &scene;
			m_entityID = entityID;
			m_persistentID = m_scene->GetRegistry().GetUUID(entityID);
		}
		~Entity() {
			m_scene = nullptr;
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
		UUID& GetUUID() { return m_persistentID; }
		Entity GetParent() {
			if (GetComponent<HierarchyMember>().parent == 0)
				return *this;
			return Entity(*m_scene,GetComponent<HierarchyMember>().parent);
		}
		void SetParent(Entity parent) {
			GetComponent<HierarchyMember>().parent = parent.m_entityID;
		}
		//GetChildren to add
		template <typename T>
		bool HasComponent() {
			return m_scene->GetRegistry().HasComponent<T>(m_entityID);
		}
	private:
		uint32_t m_entityID{};
		UUID m_persistentID;
		Scene* m_scene = nullptr;
	};
}