#include <glfw_window.hpp>
#include <image.hpp>

#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

#include <fmt/format.h>

#include <array>
#include <exception>

static void glfw_error_callback( int error, const char * description )
{
    fmt::print( "Glfw Error {}: {}\n", error, description );
}

#ifdef __EMSCRIPTEN__

EM_JS( int, js_canvas_get_width, (), { return Module.canvas.width; } );
EM_JS( int, js_canvas_get_height, (), { return Module.canvas.height; } );

#endif

namespace ui
{

void GlfwWindow::update()
{
    glfwPollEvents();

#ifdef __EMSCRIPTEN__
    // Canvas resizing does not trigger a GLFW resize callback
    display_w_ = js_canvas_get_width();
    display_h_ = js_canvas_get_height();
    glfwSetWindowSize( glfw_window_handle, display_w_, display_h_ );
#else
    glfwGetFramebufferSize( glfw_window_handle, &display_w_, &display_h_ );
#endif
}

void GlfwWindow::resize( int width, int height )
{
    float xscale = 1;
    float yscale = 1;
#ifndef __EMSCRIPTEN__
    glfwGetWindowContentScale( glfw_window_handle, &xscale, &yscale );
#endif

    this->display_w_ = width * xscale;
    this->display_h_ = height * yscale;
}

std::pair<int, int> GlfwWindow::get_size()
{
    return { display_w_, display_h_ };
}

bool GlfwWindow::should_close()
{
    return glfwWindowShouldClose( glfw_window_handle );
}

GlfwWindow::GlfwWindow( const std::string & title )
{
    glfwSetErrorCallback( glfw_error_callback );

    // Note: for a macOS .app bundle, glfwInit changes the working
    // directory to the bundle's Contents/Resources directory
    if( !glfwInit() )
    {
        fmt::print( "Failed to initialize GLFW\n" );
        // return 1;
        throw std::runtime_error( "Failed to initialize GLFW" );
    }

    glfwWindowHint( GLFW_SAMPLES, 16 ); // 16x antialiasing
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE ); // We don't want the old OpenGL
    // glfwWindowHint( GLFW_AUTO_ICONFIY, false );
    // glfwWindowHint( GLFW_DECORATED, false );
    // glfwWindowHint( GLFW_RESIZABLE, true );
#if __APPLE__
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, true );
#endif

    // Open a window and create its OpenGL context
    int canvas_width   = 1280;
    int canvas_height  = 720;
    glfw_window_handle = glfwCreateWindow( canvas_width, canvas_height, title.c_str(), NULL, NULL );
    glfwMakeContextCurrent( glfw_window_handle );
#ifndef __EMSCRIPTEN__
    glfwSwapInterval( 1 ); // Enable vsync
#endif

    if( glfw_window_handle == NULL )
    {
        fmt::print( "Failed to open GLFW window.\n" );
        glfwTerminate();
        // return -1;
        throw std::runtime_error( "Failed to open GLFW window." );
    }
}

bool GlfwWindow::set_app_icon( const std::string & icon )
{
#ifndef __EMSCRIPTEN__
    images::Image image_icon( icon );

    constexpr int NUM_CHANNELS = 4;
    constexpr std::array<int, 15> RESOLUTIONS{ 16, 20, 24, 28, 30, 31, 32, 40, 42, 47, 48, 56, 60, 63, 84 };

    std::vector<std::vector<stbir_uint8>> data( RESOLUTIONS.size() );
    std::array<GLFWimage, RESOLUTIONS.size()> glfw_images;

    if( image_icon.image_data )
    {
        for( int i = 0; i < RESOLUTIONS.size(); ++i )
        {
            glfw_images[i].pixels = new unsigned char[NUM_CHANNELS * RESOLUTIONS[i] * RESOLUTIONS[i]];
            glfw_images[i].width  = RESOLUTIONS[i];
            glfw_images[i].height = RESOLUTIONS[i];

            stbir_resize_uint8(
                image_icon.image_data, image_icon.width, image_icon.height, 0, glfw_images[i].pixels, RESOLUTIONS[i],
                RESOLUTIONS[i], 0, NUM_CHANNELS );
        }

        glfwSetWindowIcon( glfw_window_handle, int( glfw_images.size() ), glfw_images.data() );
        return true;
    }

    return false;
#else
    return true;
#endif
}

GlfwWindow::~GlfwWindow()
{
    glfwDestroyWindow( glfw_window_handle );
    glfwTerminate();
}

} // namespace ui