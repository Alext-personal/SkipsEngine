#include "pch.h"
#include "Input/Input.h"
namespace Gaze {
    Input* Input::s_instance = nullptr;
    Input::Input() {
        if (s_instance != nullptr)
        {
            ENGINE_ASSERT(0, "INPUT INSTANCE NOT SINGULAR");
        }
        s_instance = this;
    };
    void Input::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressEvent>(TO_EVENT_FN(HandleKeyPress));
        dispatcher.Dispatch<KeyReleaseEvent>(TO_EVENT_FN(HandleKeyRelease));
        dispatcher.Dispatch<MouseButtonPressEvent>(TO_EVENT_FN(HandleButtonPress));
        dispatcher.Dispatch<MouseButtonReleaseEvent>(TO_EVENT_FN(HandleButtonRelease));
        dispatcher.Dispatch<MouseScrollEvent>(TO_EVENT_FN(HandleMouseScroll));
        dispatcher.Dispatch<MouseMoveEvent>(TO_EVENT_FN(HandleMouseMove));
    }

    bool Input::HandleKeyPress(KeyPressEvent& e)
    {
        m_keysPressed[e.GetKey()] = true;
        LOG_INFO("${} Was Pressed", e.GetKey());
        return true;
    }

    bool Input::HandleKeyRelease(KeyReleaseEvent& e)
    {
        m_keysPressed[e.GetKey()] = false;
        return true;
    }
    void Input::OnFrameEnd() {
        m_previousKeys = m_keysPressed;
        m_mouse.moved = false;
        m_mouse.xdelta = 0;
        m_mouse.ydelta = 0;
    }

    bool Input::HandleButtonPress(MouseButtonPressEvent& e)
    {
        m_mouse.pressedButtons[e.GetButton()] = true;
        LOG_INFO("${} Was Pressed", e.GetButton());
        return true;
    }

    bool Input::HandleButtonRelease(MouseButtonReleaseEvent& e)
    {
        m_mouse.pressedButtons[e.GetButton()] = false;
        return true;
    }

    bool Input::HandleMouseScroll(MouseScrollEvent& e)
    {
        m_mouse.xoffset = e.GetMouseXOffset();
        m_mouse.yoffset = e.GetMouseYOffset();
        return true;
    }

    bool Input::HandleMouseMove(MouseMoveEvent& e)
    {
        float x = e.GetMouseX();
        float y = e.GetMouseY();
        if (m_mouse.firstLook) // first iteration
        {
            m_mouse.xpos = x;
            m_mouse.ypos = y;
            m_mouse.firstLook = false;
            return true;
        }
        m_mouse.xdelta += x - m_mouse.xpos;
        m_mouse.ydelta += m_mouse.ypos - y;
        m_mouse.moved = true;
        m_mouse.xpos = x;
        m_mouse.ypos = y;
        return true;
    }
}