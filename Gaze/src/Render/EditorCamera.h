#include "Scene/Component.h"
#include "Events/Event.h"
#include "Events/WindowEvents.h"
class EditorCamera {
public:
	static void OnUpdate(float dt);
	static void OnWindowResize(uint32_t width, uint32_t height);
	static const glm::mat4 GetProjectionMatrix()  { return m_camera.GetProjectionMatrix(); }
	static const glm::mat4 GetViewMatrix()  { return m_camera.GetViewMatrix(m_cameraTransform); }
private:
	static PerspectiveCameraComponent m_camera;
	static float m_cameraMoveSpeed;
	static float m_cameraSensitivity;
	static Transform m_cameraTransform;
};