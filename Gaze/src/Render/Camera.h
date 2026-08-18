#pragma once 
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace Gaze {
	class Camera {
	public:
		enum class CameraProjectionType { Ortographic, Perspective };
		Camera() { RecalculateProjectionMatrix(); }
		const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
		glm::mat4 GetViewMatrix(const Transform& transform) const {
			return glm::lookAt(transform.translation,
				transform.translation + transform.GetForward(),
				transform.GetUp());
		}
		void SetViewportSize(uint32_t width, uint32_t height) { m_aspectRatio = (float)width / (float)height; RecalculateProjectionMatrix(); }
		void SetProjectionType(CameraProjectionType type) { m_type = type; RecalculateProjectionMatrix(); }
		void SetPerspectiveFov(float fov) { m_perspectiveFov = fov; RecalculateProjectionMatrix(); }
		float GetPerspectiveFov() const { return m_perspectiveFov; }
		void SetNearPlane(float near) { m_nearPlane = near; RecalculateProjectionMatrix(); }
		void SetFarPlane(float far) { m_farPlane = far; RecalculateProjectionMatrix(); }

		void SetOrtographicSize(float size) { m_ortographicSize = size; RecalculateProjectionMatrix(); }

	private:
		float m_perspectiveFov = glm::radians(45.0f);
	private:
		float m_ortographicSize = 1080;
	private:
		float m_nearPlane = 0.01f, m_farPlane = 100.0f;
		float m_aspectRatio = 16.0f / 9.0f;
		CameraProjectionType m_type = CameraProjectionType::Perspective;
		glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
		void RecalculateProjectionMatrix() {
			if (m_type == CameraProjectionType::Perspective) {
				m_projectionMatrix = glm::perspective(m_perspectiveFov, m_aspectRatio, m_nearPlane, m_farPlane);
			}
			else
			{
				float Left = -m_ortographicSize * m_aspectRatio * 0.5f;
				float Right = m_ortographicSize * m_aspectRatio * 0.5f;
				float Bottom = -m_ortographicSize * 0.5f;
				float Top = m_ortographicSize * 0.5f;

				m_projectionMatrix = glm::ortho(Left, Right,
					Bottom, Top, m_nearPlane, m_farPlane);
			}
		}
	};
}