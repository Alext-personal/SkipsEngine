#include "Render/EditorCamera.h"
#include "Input/Input.h"
PerspectiveCameraComponent EditorCamera::m_camera{ 45.0f,16.0f / 9.0f,0.1f,100.0f };
float EditorCamera::m_cameraMoveSpeed = 1.0f;
float EditorCamera::m_cameraSensitivity = 0.05f;
Transform EditorCamera::m_cameraTransform{ glm::vec3(0.0f,0.0f,1.0f),glm::vec3(0.0f),glm::vec3(1.0f) };
void EditorCamera::OnUpdate(float dt) {
	if (Input::IsKeyPressed(KeyCode::W))
		m_cameraTransform.translation += m_cameraMoveSpeed * m_cameraTransform.GetForward() * dt;
	if (Input::IsKeyPressed(KeyCode::S))
		m_cameraTransform.translation += m_cameraMoveSpeed * (-1) * m_cameraTransform.GetForward() * dt;
	if (Input::IsKeyPressed(KeyCode::D))
		m_cameraTransform.translation += m_cameraMoveSpeed * m_cameraTransform.GetRight() * dt;
	if (Input::IsKeyPressed(KeyCode::A))
		m_cameraTransform.translation += m_cameraMoveSpeed * (-1) * m_cameraTransform.GetRight() * dt;
	if (Input::MouseMoved())
	{
		float xRotation = Input::GetMouseXDelta();
		float yRotation = Input::GetMouseYDelta();
		m_cameraTransform.rotation.y -= xRotation * m_cameraSensitivity;
		m_cameraTransform.rotation.x += yRotation * m_cameraSensitivity;
	}
}

void EditorCamera::OnWindowResize(uint32_t width,uint32_t height) {
	m_camera.aspectRatio = (float)width / (float)height;
}