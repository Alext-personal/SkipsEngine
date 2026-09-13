#include "pch.h"
#include "Scene/Scene.h"
#include "Render/Renderer.h"
#include "Render/EditorCamera.h"
#include "Resources/ResourceManager.h"
#include "Render/Material.h"
namespace Gaze {
	Scene::Scene() : m_editor(true) {
		m_systems.push_back(std::make_unique<TransformSystem>(&m_entities));
	}
	void Scene::OnUpdate(float dt) {
		for (auto& system : m_systems) {
			system->OnUpdate(dt);
		}
		OnRender();
	}
	void Scene::OnRender() {
		CameraUniformPass pass;
		if (m_editor)
			pass = { EditorCamera::GetProjectionMatrix(),EditorCamera::GetViewMatrix() };
		//todo else
		Renderer::SetUniformBuffer(pass); // once per frame
		for (auto& [transform, meshRenderer] : m_entities.Get<Transform, MeshRenderer>()) {
			Renderer::Draw(*transform, *meshRenderer);
		}
	}
	void Scene::Unload() {
		ResourceManager::Get().UnloadResources();
	}
}