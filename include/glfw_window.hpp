#pragma once
#ifndef IMGUI_APP_GLFW_WINDOW_HPP
#define IMGUI_APP_GLFW_WINDOW_HPP

#include <GLFW/glfw3.h>

#include <string>

namespace ui
{

class GlfwWindow
{
public:
    GlfwWindow( const std::string & title );

protected:
    ~GlfwWindow();

    GLFWwindow * glfw_window;
};

} // namespace ui

#endif