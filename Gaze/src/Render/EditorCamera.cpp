#include "Render/EditorCamera.h"
#include "Input/Input.h"
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
	m_camera.SetViewportSize(width ,height);
}