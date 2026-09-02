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
			if (parent.m_entityID == m_entityID)
				return;
			HierarchyMember& hierarchy = GetComponent<HierarchyMember>();
			Transform& transform = GetComponent<Transform>();
			if (transform.isDirty)
				m_scene->GetTransformSystem()->ResolveEntity(m_entityID);
			glm::mat4 currentWorldTransform = transform.GetMatrix();
			if (hierarchy.parent != 0) {
				Entity oldParent = GetParent();
				oldParent.RemoveChild(*this);
			}
			if (parent.m_entityID != 0) {
				parent.AddChild(*this);
				Transform& parentTransform = parent.GetComponent<Transform>();
				if(parentTransform.isDirty)
					m_scene->GetTransformSystem()->ResolveEntity(parent.m_entityID);
				glm::mat4 newParentWorld = parentTransform.GetMatrix();
				glm::mat4 newLocal = glm::inverse(newParentWorld) * currentWorldTransform;
				transform.SetFromMatrix(newLocal);
			}
			else
				transform.SetFromMatrix(currentWorldTransform);
			hierarchy.parent = parent.m_entityID;
		}
		void AddChild(Entity child) {
			GetComponent<HierarchyMember>().children.push_back(child.m_entityID);
		}
		void RemoveChild(Entity child) {
			HierarchyMember& hierarchy = GetComponent<HierarchyMember>();
			auto it = std::find(hierarchy.children.begin(), hierarchy.children.end(), child.m_entityID);
			if (it != hierarchy.children.end())
				hierarchy.children.erase(it);
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