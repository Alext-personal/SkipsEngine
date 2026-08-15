#include "Scene/Scene.h"
#include "Render/Renderer.h"
#include "Render/EditorCamera.h"
Scene::Scene(): m_editor(true) {
}
void Scene::OnRender() {
	CameraUniformPass pass;
	if (m_editor)
		pass = { EditorCamera::GetProjectionMatrix(),EditorCamera::GetViewMatrix() };
	//todo else
	Renderer::SetUniformBuffer(pass);
	for (auto& [transform, meshRenderer] : m_entities.Get<Transform, MeshRenderer>()) {
		Renderer::Draw(transform, *meshRenderer.mesh, *meshRenderer.shader);
	}
}
uint32_t Scene::AddEntity() {
	uint32_t entity = m_entities.CreateEntity();
	Transform& transform = m_entities.AddComponent<Transform>(entity);
	MeshRenderer& mrenderer = m_entities.AddComponent<MeshRenderer>(entity);
	transform.scale = { .25f,.25f,.25f };
	mrenderer.LoadMesh("Gaze/assets/models/Car.obj");

	return entity;
}