#pragma once
#include "Scene/Scene.h"
namespace Gaze {
	class Entity {
	public:
		Entity(Scene& scene) {
			m_scene = &scene;
			m_entityID = m_scene->GetRegistry().CreateEntity();
			AddComponent<Transform>(); // every entity has a transform so far
		}
		~Entity() {
			m_scene->GetRegistry().DeleteEntity(m_entityID);
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
		template <typename T>
		bool HasComponent() {
			return m_scene->GetRegistry().HasComponent<T>(m_entityID);
		}
	private:
		uint32_t m_entityID{};
		Scene* m_scene = nullptr;
	};
}