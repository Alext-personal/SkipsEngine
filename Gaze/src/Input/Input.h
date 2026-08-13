#pragma once
#include "Events/InputEvents.h"
#include <array>
class Input {
public:
	Input();
	~Input() = default;
	void OnEvent(Event &e);
	void OnFrameEnd();
	static Input& GetInstance() { return *s_instance; }

	//handlers
	bool HandleKeyPress(KeyPressEvent &e);
	bool HandleKeyRelease(KeyReleaseEvent &e);
	bool HandleButtonPress(MouseButtonPressEvent &e);
	bool HandleButtonRelease(MouseButtonReleaseEvent &e);
	bool HandleMouseScroll(MouseScrollEvent &e);
	bool HandleMouseMove(MouseMoveEvent &e);

	//static getters

	static double GetMouseX() { return GetInstance().m_mouse.xpos; }
	static double GetMouseY() { return GetInstance().m_mouse.ypos; }
	static double GetMouseXDelta() { return GetInstance().m_mouse.xdelta; }
	static double GetMouseYDelta() { return GetInstance().m_mouse.ydelta; }
	static double GetScrollX() { return GetInstance().m_mouse.xoffset; }
	static double GetScrollY() { return GetInstance().m_mouse.yoffset; }
	static bool MouseMoved() { return GetInstance().m_mouse.moved; }
	static bool IsKeyPressed(const KeyCode keycode) { return  GetInstance().m_keysPressed[keycode]; }
	static bool IsKeyTapped(const KeyCode keycode) { return !GetInstance().m_previousKeys[keycode] && GetInstance().m_keysPressed[keycode]; }
	static bool IsMouseButtonPressed(const MouseCode mousecode) { return  GetInstance().m_mouse.pressedButtons[mousecode]; }

private:
	static Input* s_instance;
	std::array<bool,350> m_keysPressed{};
	std::array<bool, 350> m_previousKeys{};
	struct Mouse {
		double xpos, ypos; //cursor
		double xdelta, ydelta; // cursor
		double xoffset, yoffset; //scroll
		std::array<bool, 10> pressedButtons;
		bool firstLook = true;
		bool moved = false;
	};
	Mouse m_mouse{};
};