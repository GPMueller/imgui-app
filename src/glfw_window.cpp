#include <glfw_window.hpp>

#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <fmt/format.h>

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
    display_w = js_canvas_get_width();
    display_h = js_canvas_get_height();
    glfwSetWindowSize( glfw_window_handle, display_w, display_h );
#else
    glfwGetFramebufferSize( glfw_window_handle, &display_w, &display_h );
#endif
}

void GlfwWindow::resize( int width, int height )
{
    float xscale = 1;
    float yscale = 1;
#ifndef __EMSCRIPTEN__
    glfwGetWindowContentScale( glfw_window_handle, &xscale, &yscale );
#endif

    this->display_w = width * xscale;
    this->display_h = height * yscale;
}

std::pair<int, int> GlfwWindow::get_size()
{
    return { display_w, display_h };
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

GlfwWindow::~GlfwWindow()
{
    glfwDestroyWindow( glfw_window_handle );
    glfwTerminate();
}

} // namespace ui