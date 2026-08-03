#include "Event.h"
class WindowCloseEvent : public Event {
public:
	WindowCloseEvent() = default;
	EVENT_CLASS_TYPE(WindowClose);
};
class WindowResizeEvent : public Event {
public:
	WindowResizeEvent(unsigned int width, unsigned int height) : m_width(width), m_height(height) {}
	unsigned int GetWidth() const { return m_width; }
	unsigned int GetHeight() const { return m_height; }
	EVENT_CLASS_TYPE(WindowResize)
private:
	unsigned int m_width{};
	unsigned int m_height{};
};