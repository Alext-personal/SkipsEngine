#pragma once
#include "Scene/EntityRegistry.h"
#include "Scene/EntitySystems.h"
#include "Render/Camera.h"
namespace Gaze {
	class Scene {
	public:
		Scene();
		void OnUpdate(float dt);
		void OnRender();
		EntityRegistry& GetRegistry() { return m_entities; }
		TransformSystem* GetTransformSystem() { return static_cast<TransformSystem*>(m_systems[0].get()); }
	private:
		EntityRegistry m_entities{};
		std::unique_ptr<Camera> m_activeCamera;
		std::vector<std::unique_ptr<EntitySystem>> m_systems;
		bool m_editor;
	};
}