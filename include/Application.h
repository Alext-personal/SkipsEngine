#include <memory>
#include "Event.h"
#include "WindowEvents.h"
#include "Window.h"
class Application {
public:
	Application();
	~Application() = default;

	Application& Get() const { return *s_instance; }
	void OnEvent(Event& e);
	void OnWindowClose(WindowCloseEvent& e);
	void OnWindowResize(WindowResizeEvent& e);
	void Run();

private:
	static Application* s_instance;

	std::unique_ptr<Window> m_window;
	bool m_running = true;
};