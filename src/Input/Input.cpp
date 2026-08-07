#include "core/Helpers.h"
#include "core/Log.h"
#include "input/Input.h"

Input* Input::s_instance = nullptr;
Input::Input() { 
    if (s_instance != nullptr)
    {
        LOG_ERROR("INPUT INSTANCE NOT SINGULAR");
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
    Log::INFO(e.GetKey(), " Was Pressed");
    return true;
}

bool Input::HandleKeyRelease(KeyReleaseEvent& e)
{
    m_keysPressed[e.GetKey()] = false;
    return true;
}

bool Input::HandleButtonPress(MouseButtonPressEvent& e)
{
    m_mouse.pressedButtons[e.GetButton()] = true;
    Log::INFO(e.GetButton(), " Was Pressed");
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
    m_mouse.xpos = e.GetMouseX();
    m_mouse.ypos = e.GetMouseY();
    return true;
}