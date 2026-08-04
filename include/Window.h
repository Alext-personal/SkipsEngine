#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <string>
class Event;
class Window {
public:
	using EventCallbackFn = std::function<void(Event&)>;
	Window(unsigned int width, unsigned int height, const char *name);
	~Window();

	bool ShouldClose() const;

	void SetVSync(bool enabled);
	bool IsVSync() const { return m_data.vsync; }

	int GetWidth() const { return m_data.width; }
	int GetHeight() const { return m_data.height; }
	GLFWwindow* GetNativeWindow() const { return m_window; }

	void SetCallbackFunction(EventCallbackFn func) { m_data.EventCallback = func; }

	void SwapBuffers();
	void PollEvents();
private:
	GLFWwindow* m_window;
	struct WindowData {
		std::string title;
		unsigned int width, height;
		bool vsync;
		EventCallbackFn EventCallback;
	};
	WindowData m_data{};
};