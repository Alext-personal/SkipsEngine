#include <glad/glad.h>
#include "core/Log.h"
#include <GLFW/glfw3.h>
#include "render/Renderer.h"

void Renderer::Init() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		Log::ERROR("Error at glad initialisation");
}