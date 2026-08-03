#include <functional>
enum class EventType {
	WindowClose, WindowResize
};
#define EVENT_CLASS_TYPE(type)\
	static EventType GetStaticType(){return EventType::type;}\
	virtual EventType GetEventType() const override {return GetStaticType();}\
	virtual const char * GetName() const override {return #type;}

class Event {
public:
	bool handled = false;
	virtual ~Event() = default;
	Event() = default;
	virtual EventType GetEventType() const = 0;
	virtual const char* GetName() const = 0;
};
class EventDispatcher {
public:
	EventDispatcher(Event& e) : m_event(e) {};
	using EventFn = std::function<bool(Event&)>;
	template<typename T>
	bool Dispatch(const EventFn& func) {
		if (m_event.GetEventType == T::GetStaticType()) {
			m_event.handled |= func(static_cast<T*>(m_event));
			return true;
		}
		return false;
	}
private:
	Event& m_event;
};