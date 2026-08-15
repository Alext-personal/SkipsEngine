#include "Scene/EntityRegistry.h"
#include "Render/Camera.h"
class Scene {
public:
	Scene();
	void OnRender();
	uint32_t AddEntity();
private:
	EntityRegistry m_entities{};
	std::unique_ptr<Camera> m_activeCamera;
	bool m_editor;
};