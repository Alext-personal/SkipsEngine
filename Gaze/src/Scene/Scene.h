#pragma once
#include "Scene/EntityRegistry.h"
#include "Render/Camera.h"
class Scene {
public:
	Scene();
	void OnRender();
	EntityRegistry& GetRegistry(){ return m_entities; }
private:
	EntityRegistry m_entities{};
	std::unique_ptr<Camera> m_activeCamera;
	bool m_editor;
};