#pragma once
#ifndef IMGUI_APP_APPLICATION_HPP
#define IMGUI_APP_APPLICATION_HPP

#include <glfw_window.hpp>

#include <imgui/imgui.h>

#include <string>

namespace ui
{

class Application : GlfwWindow
{
public:
    Application( const std::string & title );
    ~Application();

    void run();
    void draw();

protected:
    void draw_gl();
    void draw_menu_bar();
    void draw_overlay();

    ImVec4 clear_color  = ImVec4( 0.4f, 0.4f, 0.4f, 1.f );
    bool gl_initialized = false;
    float gradient      = 1.0f;

    bool show_demo_window    = false;
    bool show_another_window = false;
    bool show_overlay        = true;
    int selected_mode        = 1;

    bool dark_mode = true;

    int display_w = 100;
    int display_h = 100;

    ImFont * font_cousine = nullptr;
    ImFont * font_12      = nullptr;
    ImFont * font_14      = nullptr;
    ImFont * font_16      = nullptr;
    ImFont * font_18      = nullptr;
};

} // namespace ui

#endif