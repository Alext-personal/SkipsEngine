#pragma once
#include <GLFW/glfw3.h>
class Window {
public:
	Window(int width, int height, const char *name);
	~Window();
	bool ShouldClose() const;
	void SwapBuffers();
	void PollEvents();
	void SetFrameBufferSizeCallback(GLFWframebuffersizefun func); //temporary
private:
	GLFWwindow* m_window;
};