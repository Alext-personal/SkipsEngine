#pragma once
#include "Events/Event.h"
#include <string>
namespace Gaze {
	class Layer {
	public:
		Layer(const std::string& layername = "Default");
		virtual ~Layer();
		virtual void OnUpdate(float dt) {}
		virtual void OnEvent(Event& e) {}
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnImguiRender(){}
		inline const std::string GetName() const { return m_name; }
	protected:
		std::string m_name;
	};
}