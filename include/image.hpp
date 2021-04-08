#pragma once
#ifndef PLANNER_UTILITY_IMAGES_HPP
#define PLANNER_UTILITY_IMAGES_HPP

#include <GLFW/glfw3.h>

#include <string>

namespace images
{

struct Image
{
    Image( const std::string_view file_name );
    ~Image();

    // Load the image into an OpenGL texture
    bool get_gl_texture( unsigned int & out_texture );

    std::string file_name = "";

    std::string file_data      = "";
    int width                  = 0;
    int height                 = 0;
    unsigned char * image_data = nullptr;
};

bool glfw_set_app_icon( GLFWwindow * glfw_window );

} // namespace images

#endif