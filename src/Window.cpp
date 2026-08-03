#include <iostream>
#include <glad/glad.h>
#include "Window.h"
#include "WindowEvents.h"
Window::Window(unsigned int width, unsigned int height, const char * name) {
	if (!glfwInit()) {
		throw std::runtime_error("Error at GLFW initialisation");
	}
	m_window = glfwCreateWindow(width, height, name, NULL, NULL);

	if (!m_window) {
		glfwTerminate();
		throw std::runtime_error("Error at window creation");
	}
	glfwMakeContextCurrent(m_window);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwTerminate();
		throw std::runtime_error("Error at glad initialisation");
	}
	//WindowData

	m_data.title = name;
	m_data.width = width;
	m_data.height = height;
	m_data.vsync = true;

	glfwSetWindowUserPointer(m_window, &m_data);
	SetVSync(m_data.vsync);

	//callbacks

	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
		WindowData data = *(WindowData*)glfwGetWindowUserPointer(window);
		WindowCloseEvent event;
		data.EventCallback(event);
		});
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		WindowData data = *(WindowData*)glfwGetWindowUserPointer(window);
		WindowResizeEvent event(width,height);
		data.width = width;
		data.height = height;
		data.EventCallback(event);
		});
}
Window::~Window() {
	if (m_window)
		glfwDestroyWindow(m_window);
	glfwTerminate();
}
bool Window::ShouldClose() const {
	return glfwWindowShouldClose(m_window);
}
void Window::SetVSync(bool enabled){
	glfwSwapInterval(enabled);
	m_data.vsync = enabled;
}
void Window::SwapBuffers(){
	glfwSwapBuffers(m_window);
}
void Window::PollEvents() {
	glfwPollEvents();
}