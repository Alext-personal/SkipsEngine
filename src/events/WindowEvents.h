#pragma once
#include "events/Event.h"
class WindowCloseEvent : public Event {
public:
	WindowCloseEvent() = default;
	EVENT_CLASS_TYPE(WindowClose);
	EVENT_CLASS_CATEGORY(WindowEvent);
};
class WindowResizeEvent : public Event {
public:
	WindowResizeEvent(unsigned int width, unsigned int height) : m_width(width), m_height(height) {}
	unsigned int GetWidth() const { return m_width; }
	unsigned int GetHeight() const { return m_height; }
	EVENT_CLASS_TYPE(WindowResize);
	EVENT_CLASS_CATEGORY(WindowEvent);
private:
	unsigned int m_width{};
	unsigned int m_height{};
};