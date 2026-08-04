#pragma once
#include "Event.h"
#include "KeyCodes.h"
#include "MouseCodes.h"

// Key Events
class KeyPressEvent : public Event {
public:
	KeyPressEvent(KeyCode key, bool repeat): m_keyCode(key) , m_isRepeat(repeat){}

	KeyCode GetKey() const { return m_keyCode };
	bool IsRepeat() const { return m_isRepeat };

	EVENT_CLASS_TYPE(EventType::KeyPress);
	EVENT_CLASS_CATEGORY(EventCategory::KeyEvent);
private:
	KeyCode m_keyCode;
	bool m_isRepeat;
};
class KeyReleaseEvent : public Event {
	KeyReleaseEvent(KeyCode key) : m_keyCode(key) {}

	KeyCode GetKey() const { return m_keyCode };

	EVENT_CLASS_TYPE(EventType::KeyRelease);
	EVENT_CLASS_CATEGORY(EventCategory::KeyEvent);
private:
	KeyCode m_keyCode;
};
// Mouse Events
class MouseCursorEvent : public Event{};
class MouseScrollEvent : public Event{};
class MouseButtonPressEvent : public Event{};
class MouseButtonReleaseEvent : public Event {};