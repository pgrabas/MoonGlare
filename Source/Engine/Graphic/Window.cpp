/*
    Generated by cppsrc.sh
    On 2014-12-13 13:22:00,37
    by Paweu
*/

#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/Engine.h>
#include <Engine/iApplication.h>
#include "GraphicSettings.h"

#include <Core/InputProcessor.h>

namespace Graphic {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(Window)

Window::Window(GLFWwindow *w):
        BaseClass(),
        m_Window(w), 
        m_Size(0) {
    int iw, ih;
    glfwGetWindowSize(m_Window, &iw, &ih);
    m_Size = uvec2(iw, ih);
}

//-------------------------------------------------------------------------------------------------

unsigned Window::GetRefreshRate() const {
    auto monitor = glfwGetWindowMonitor(m_Window);
    if (!monitor)
        monitor = glfwGetPrimaryMonitor();
    return glfwGetVideoMode(monitor)->refreshRate;
}

//-------------------------------------------------------------------------------------------------

} //namespace Graphic 
