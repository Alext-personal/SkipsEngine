#pragma once
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
struct Transform {
	glm::vec3 Translation{ 0.0f };
	glm::vec3 Rotation{ 0.0f };
	glm::vec3 Scale{ 1.0f };
};