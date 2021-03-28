#pragma once
#ifndef IMGUI_APP_APPLICATION_HPP
#define IMGUI_APP_APPLICATION_HPP

#include <glfw_window.hpp>
#include <opengl_renderer.hpp>

#include <imgui/imgui.h>

#include <string>

namespace ui
{

class Application
{
public:
    Application( const std::string & title );
    ~Application();

    void run();
    void draw();

    void resize( int width, int height );

protected:
    void draw_menu_bar();
    void draw_overlay();

    bool show_demo_window    = false;
    bool show_another_window = false;
    bool show_overlay        = true;
    int selected_mode        = 1;

    bool dark_mode = true;

    ImFont * font_cousine = nullptr;
    ImFont * font_12      = nullptr;
    ImFont * font_14      = nullptr;
    ImFont * font_16      = nullptr;
    ImFont * font_18      = nullptr;

    GlfwWindow glfw_window;
    OpenglRenderer opengl_renderer;
};

} // namespace ui

#endif