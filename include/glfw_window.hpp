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
    bool set_app_icon( const std::string & icon );
    void resize( int width, int height );
    std::pair<int, int> get_size();

    ~GlfwWindow();
    void update();
    bool should_close();

    GLFWwindow * glfw_window_handle;

private:
    int display_w_ = 100;
    int display_h_ = 100;
};

} // namespace ui

#endif