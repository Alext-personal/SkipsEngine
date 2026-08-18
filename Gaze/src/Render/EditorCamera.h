#pragma once
#include "Scene/Component.h"
#include "Events/Event.h"
#include "Events/WindowEvents.h"
#include "Render/Camera.h"
class EditorCamera {
public:
	static void OnUpdate(float dt);
	static void OnWindowResize(uint32_t width, uint32_t height);
	static const Camera& GetCamera() { return m_camera; }
	static const glm::mat4& GetProjectionMatrix() { return m_camera.GetProjectionMatrix(); }
	static const glm::mat4 GetViewMatrix() { return m_camera.GetViewMatrix(m_cameraTransform); }
private:
	inline static Camera m_camera{};
	inline static float m_cameraMoveSpeed{ 1.0f };
	inline static float m_cameraSensitivity{ 0.05f };
	inline static Transform m_cameraTransform{ glm::vec3(0.0f,0.0f,1.0f),glm::vec3(0.0f),glm::vec3(1.0f) };
};