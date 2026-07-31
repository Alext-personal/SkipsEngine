#include <iostream>
#include "Window.h"
Window::Window(int width, int height, const char * name) {
	if (!glfwInit()) {
		throw std::runtime_error("Error at GLFW initialisation");
	}
	m_window = glfwCreateWindow(width, height, name, NULL, NULL);

	if (!m_window) {
		glfwTerminate();
		throw std::runtime_error("Error at window creation");
	}
	glfwMakeContextCurrent(m_window);
}
Window::~Window() {
	if (m_window)
		glfwDestroyWindow(m_window);
	glfwTerminate();
}
bool Window::ShouldClose() const {
	return glfwWindowShouldClose(m_window);
}
void Window::SwapBuffers(){
	glfwSwapBuffers(m_window);
}
void Window::PollEvents() {
	glfwPollEvents();
}