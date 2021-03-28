#pragma once
#ifndef IMGUI_APP_GLFW_WINDOW_HPP
#define IMGUI_APP_GLFW_WINDOW_HPP

#include <string>
#include <utility>

struct GLFWwindow;

namespace ui
{

class GlfwWindow
{
public:
    GlfwWindow( const std::string & title );
    void resize( int width, int height );
    std::pair<int, int> get_size();

    ~GlfwWindow();
    void update();
    bool should_close();

    GLFWwindow * glfw_window_handle;

private:
    int display_w = 100;
    int display_h = 100;
};

} // namespace ui

#endif