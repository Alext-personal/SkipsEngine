#pragma once
#include "InputEvents.h"
#include <array>
class Input {
public:
	Input() = default;
	~Input() = default;
	void OnEvent(Event &e);
	//handlers
	bool HandleKeyPress(KeyPressEvent &e);
	bool HandleKeyRelease(KeyReleaseEvent &e);
	bool HandleButtonPress(MouseButtonPressEvent &e);
	bool HandleButtonRelease(MouseButtonReleaseEvent &e);
	bool HandleMouseScroll(MouseScrollEvent &e);
	bool HandleMouseMove(MouseMoveEvent &e);

	//getters
	double GetMouseX() const { return m_mouse.xpos; }
	double GetMouseY() const { return m_mouse.ypos; }
	double GetScrollX() const { return m_mouse.xoffset; }
	double GetScrollY() const { return m_mouse.yoffset; }
	bool IsKeyPressed(const KeyCode keycode) const { return m_keysPressed[keycode]; }
	bool IsMouseButtonPressed(const MouseCode mousecode) const { return m_mouse.pressedButtons[mousecode]; }

private:
	std::array<bool,350> m_keysPressed{};
	struct Mouse {
		double xpos, ypos;
		double xoffset, yoffset;
		std::array<bool, 10> pressedButtons;
	};
	Mouse m_mouse{};
};