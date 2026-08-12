#pragma once
#include "Events/Event.h"
#include "input/KeyCodes.h"
#include "input/MouseCodes.h"

// Key Events
class KeyPressEvent : public Event {
public:
	KeyPressEvent(KeyCode key, bool repeat): m_keyCode(key) , m_isRepeat(repeat){}

	KeyCode GetKey() const { return m_keyCode; }
	bool IsRepeat() const { return m_isRepeat; }

	EVENT_CLASS_TYPE(KeyPress);
	EVENT_CLASS_CATEGORY(KeyEvent);
private:
	KeyCode m_keyCode;
	bool m_isRepeat;
};
class KeyReleaseEvent : public Event {
public:
	KeyReleaseEvent(const KeyCode key) : m_keyCode(key) {}

	KeyCode GetKey() const { return m_keyCode; }

	EVENT_CLASS_TYPE(KeyRelease);
	EVENT_CLASS_CATEGORY(KeyEvent);
private:
	KeyCode m_keyCode;
};
// Mouse Events
class MouseMoveEvent : public Event{
public:
	MouseMoveEvent(double xpos,double ypos) :m_xpos(xpos),m_ypos(ypos){}
	double GetMouseX() const { return m_xpos; }
	double GetMouseY() const { return m_ypos; }

	EVENT_CLASS_TYPE(MouseMove);
	EVENT_CLASS_CATEGORY(MouseEvent);
private:
	double m_xpos, m_ypos;
};
class MouseScrollEvent : public Event{
public:
	MouseScrollEvent(double xoffset,double yoffset): m_xoffset(xoffset),m_yoffset(yoffset){}
	double GetMouseXOffset() const { return m_xoffset; }
	double GetMouseYOffset() const { return m_yoffset; }

	EVENT_CLASS_TYPE(MouseScroll);
	EVENT_CLASS_CATEGORY(MouseEvent);
private:
	double m_xoffset, m_yoffset;
};
class MouseButtonPressEvent : public Event{
public:
	MouseButtonPressEvent(const MouseCode button) : m_button(button){}
	MouseCode GetButton() const { return m_button; }

	EVENT_CLASS_TYPE(MouseButtonPress);
	EVENT_CLASS_CATEGORY(MouseEvent);
private:
	MouseCode m_button{};
};
class MouseButtonReleaseEvent : public Event {
public:
	MouseButtonReleaseEvent(const MouseCode button) : m_button(button) {}
	MouseCode GetButton() const { return m_button; }

	EVENT_CLASS_TYPE(MouseButtonRelease);
	EVENT_CLASS_CATEGORY(MouseEvent);
private:
	MouseCode m_button{};
};