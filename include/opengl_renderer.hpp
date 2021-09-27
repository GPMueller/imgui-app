#pragma once
#ifndef IMGUI_APP_OPENGL_RENDERER_HPP
#define IMGUI_APP_OPENGL_RENDERER_HPP

namespace ui
{

class OpenglRenderer
{
public:
    void draw();
    void set_framebuffer_size( int w, int h );

    float gradient = 1;
    float background_color[4]{ 0.4f, 0.4f, 0.4f, 1.0f };

private:
    void initialize_gl();

    bool gl_initialized_         = false;
    unsigned int shader_program_ = -1;

    int framebuffer_w_ = 100;
    int framebuffer_h_ = 100;
};

} // namespace ui

#endif