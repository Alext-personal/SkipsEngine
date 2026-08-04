#pragma once
#include <functional>
enum class EventType {
	WindowClose, WindowResize,
	KeyPress,KeyRelease
};
enum EventCategory {
	MouseEvent = 1,
	KeyEvent = 1<<2,
	WindowEvent = 1<<3
};
#define EVENT_CLASS_TYPE(type)\
	static EventType GetStaticType(){return EventType::type;}\
	virtual EventType GetEventType() const override {return GetStaticType();}\
	virtual const char * GetName() const override {return #type;}
#define EVENT_CLASS_CATEGORY(category)\
	virtual int GetEventCategory() const override {return category};

class Event {
public:
	bool handled = false;
	virtual ~Event() = default;
	Event() = default;
	virtual EventType GetEventType() const = 0;
	virtual int GetEventCategory() const = 0;
	virtual const char* GetName() const = 0;

	bool IsInCategory(EventCategory category) { return GetEventCategory & category; }
};
class EventDispatcher {
public:
	EventDispatcher(Event& e) : m_event(e) {};
	template<typename T,typename F>
	bool DispatchByType(const F& func) {
		if (m_event.GetEventType() == T::GetStaticType()) {
			m_event.handled |= func(static_cast<T&>(m_event));
			return true;
		}
		return false;
	}
private:
	Event& m_event;
};