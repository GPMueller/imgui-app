#include <fonts.hpp>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <fmt/format.h>

#include <stdio.h>
#include <cmath>

GLFWwindow * g_window;

static ImVec4 clear_color       = ImVec4( 0.4f, 0.4f, 0.4f, 1.f );
static bool show_demo_window    = false;
static bool show_another_window = false;
static int selected_mode        = 1;

static bool dark_mode = true;

static bool drag_main_window = false;
static double wx_start, wy_start;
static double cx_start, cy_start;
static bool main_window_maximized = false;

static ImFont * font_14 = nullptr;
static ImFont * font_16 = nullptr;
static ImFont * font_18 = nullptr;

static bool gl_initialized = false;

#ifdef __EMSCRIPTEN__

EM_JS( int, js_canvas_get_width, (), { return Module.canvas.width; } );
EM_JS( int, js_canvas_get_height, (), { return Module.canvas.height; } );
EM_JS( void, js_resize_canvas, (), { resizeCanvas(); } );
EM_JS( void, js_notify, ( const char * message, int length ), { notifyMessage( UTF8ToString( message, length ) ); } );

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_imgui;
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_render;

#endif

static void notification( const std::string & message )
{
#ifdef __EMSCRIPTEN__
    js_notify( message.c_str(), message.size() );
#else
    // TODO
#endif
}

static void glfw_error_callback( int error, const char * description )
{
    fmt::print( "Glfw Error {}: {}\n", error, description );
}

static void draw_gl( int display_w, int display_h )
{
    // An array of 3 vectors which represents 3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
    static GLuint shaderProgram;
    if( !gl_initialized )
    {
        // Shader sources
        const GLchar * vertexSource = "attribute vec4 position;                     \n"
                                      "void main()                                  \n"
                                      "{                                            \n"
                                      "  gl_Position = vec4(position.xyz, 1.0);     \n"
                                      "}                                            \n";
        const GLchar * fragmentSource = "precision mediump float;\n"
                                        "void main()                                  \n"
                                        "{                                            \n"
                                        "  //gl_FragColor[0] = gl_FragCoord.x*200.0;    \n"
                                        "  //gl_FragColor[1] = gl_FragCoord.y*200.0;    \n"
                                        "  //gl_FragColor[2] = 0.5;                     \n"
                                        "  gl_FragColor = vec4(0.149,0.141,0.912,1.0);\n"
                                        "}                                            \n";

        // Create a Vertex Buffer Object and copy the vertex data to it
        GLuint vbo;
        glGenBuffers( 1, &vbo );

        GLfloat vertices[] = { 0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f };

        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

        // Create and compile the vertex shader
        GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
        glShaderSource( vertexShader, 1, &vertexSource, nullptr );
        glCompileShader( vertexShader );

        // Create and compile the fragment shader
        GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
        glShaderSource( fragmentShader, 1, &fragmentSource, nullptr );
        glCompileShader( fragmentShader );

        // Link the vertex and fragment shader into a shader program
        shaderProgram = glCreateProgram();
        glAttachShader( shaderProgram, vertexShader );
        glAttachShader( shaderProgram, fragmentShader );
        glLinkProgram( shaderProgram );
        glUseProgram( shaderProgram );

        gl_initialized = true;
    }

    glViewport( 0, 0, display_w, display_h );
    glClearColor( clear_color.x, clear_color.y, clear_color.z, clear_color.w );
    glClear( GL_COLOR_BUFFER_BIT );

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation( shaderProgram, "position" );
    glEnableVertexAttribArray( posAttrib );
    glVertexAttribPointer( posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0 );

    glUseProgram( shaderProgram );

    // Draw a triangle from the 3 vertices
    glDrawArrays( GL_TRIANGLES, 0, 3 );
}

void apply_charcoal_style( ImGuiStyle * dst = NULL )
{
    ImGuiStyle * style = dst ? dst : &ImGui::GetStyle();
    ImVec4 * colors    = style->Colors;

    colors[ImGuiCol_Text]                 = ImVec4( 1.000f, 1.000f, 1.000f, 1.000f );
    colors[ImGuiCol_TextDisabled]         = ImVec4( 0.500f, 0.500f, 0.500f, 1.000f );
    colors[ImGuiCol_WindowBg]             = ImVec4( 0.180f, 0.180f, 0.180f, 1.000f );
    colors[ImGuiCol_ChildBg]              = ImVec4( 0.280f, 0.280f, 0.280f, 0.000f );
    colors[ImGuiCol_PopupBg]              = ImVec4( 0.313f, 0.313f, 0.313f, 1.000f );
    colors[ImGuiCol_Border]               = ImVec4( 0.266f, 0.266f, 0.266f, 1.000f );
    colors[ImGuiCol_BorderShadow]         = ImVec4( 0.000f, 0.000f, 0.000f, 0.000f );
    colors[ImGuiCol_FrameBg]              = ImVec4( 0.160f, 0.160f, 0.160f, 1.000f );
    colors[ImGuiCol_FrameBgHovered]       = ImVec4( 0.200f, 0.200f, 0.200f, 1.000f );
    colors[ImGuiCol_FrameBgActive]        = ImVec4( 0.280f, 0.280f, 0.280f, 1.000f );
    colors[ImGuiCol_TitleBg]              = ImVec4( 0.148f, 0.148f, 0.148f, 1.000f );
    colors[ImGuiCol_TitleBgActive]        = ImVec4( 0.148f, 0.148f, 0.148f, 1.000f );
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4( 0.148f, 0.148f, 0.148f, 1.000f );
    colors[ImGuiCol_MenuBarBg]            = ImVec4( 0.195f, 0.195f, 0.195f, 1.000f );
    colors[ImGuiCol_ScrollbarBg]          = ImVec4( 0.160f, 0.160f, 0.160f, 1.000f );
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4( 0.277f, 0.277f, 0.277f, 1.000f );
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.300f, 0.300f, 0.300f, 1.000f );
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
    colors[ImGuiCol_CheckMark]            = ImVec4( 1.000f, 1.000f, 1.000f, 1.000f );
    colors[ImGuiCol_SliderGrab]           = ImVec4( 0.391f, 0.391f, 0.391f, 1.000f );
    colors[ImGuiCol_SliderGrabActive]     = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
    colors[ImGuiCol_Button]               = ImVec4( 1.000f, 1.000f, 1.000f, 0.000f );
    colors[ImGuiCol_ButtonHovered]        = ImVec4( 1.000f, 1.000f, 1.000f, 0.156f );
    colors[ImGuiCol_ButtonActive]         = ImVec4( 1.000f, 1.000f, 1.000f, 0.391f );
    colors[ImGuiCol_Header]               = ImVec4( 0.313f, 0.313f, 0.313f, 1.000f );
    colors[ImGuiCol_HeaderHovered]        = ImVec4( 0.469f, 0.469f, 0.469f, 1.000f );
    colors[ImGuiCol_HeaderActive]         = ImVec4( 0.469f, 0.469f, 0.469f, 1.000f );
    colors[ImGuiCol_Separator]            = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]     = ImVec4( 0.391f, 0.391f, 0.391f, 1.000f );
    colors[ImGuiCol_SeparatorActive]      = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
    colors[ImGuiCol_ResizeGrip]           = ImVec4( 1.000f, 1.000f, 1.000f, 0.250f );
    colors[ImGuiCol_ResizeGripHovered]    = ImVec4( 1.000f, 1.000f, 1.000f, 0.670f );
    colors[ImGuiCol_ResizeGripActive]     = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
    colors[ImGuiCol_Tab]                  = colors[ImGuiCol_Header];
    colors[ImGuiCol_TabHovered]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]            = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TabUnfocused]         = colors[ImGuiCol_Tab];
    colors[ImGuiCol_TabUnfocusedActive]   = colors[ImGuiCol_TabActive];
    colors[ImGuiCol_PlotLines]            = ImVec4( 0.469f, 0.469f, 0.469f, 1.000f );
    colors[ImGuiCol_PlotLinesHovered]     = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
    colors[ImGuiCol_PlotHistogram]        = ImVec4( 0.586f, 0.586f, 0.586f, 1.000f );
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
    colors[ImGuiCol_TextSelectedBg]       = ImVec4( 1.000f, 1.000f, 1.000f, 0.156f );
    colors[ImGuiCol_DragDropTarget]       = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );
    colors[ImGuiCol_NavHighlight]         = ImVec4( 1.000f, 0.391f, 0.000f, 1.000f );

    style->ChildRounding     = 4.0f;
    style->FrameBorderSize   = 1.0f;
    style->FrameRounding     = 2.0f;
    style->GrabMinSize       = 7.0f;
    style->PopupRounding     = 2.0f;
    style->ScrollbarRounding = 12.0f;
    style->ScrollbarSize     = 13.0f;
    style->WindowRounding    = 4.0f;
}

static void show_menu_bar( GLFWwindow * window )
{
    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 7.f, 7.f ) );
    ImGui::PushFont( font_16 );

    if( ImGui::BeginMainMenuBar() )
    {
        // mainmenu_height = ImGui::GetWindowSize().y;
        ImGui::PopStyleVar();

        // ImGui::SameLine();
        if( ImGui::BeginMenu( "File" ) )
        {
            if( ImGui::MenuItem( "New" ) )
            {
            }
            if( ImGui::MenuItem( "Open", "Ctrl+O" ) )
            {
            }
            ImGui::EndMenu();
        }
        if( ImGui::BeginMenu( "Edit" ) )
        {
            if( ImGui::MenuItem( "Undo", "CTRL+Z" ) )
            {
            }
            if( ImGui::MenuItem( "Redo", "CTRL+Y", false, false ) )
            {
            } // Disabled item
            ImGui::Separator();
            if( ImGui::MenuItem( "Cut", "CTRL+X" ) )
            {
            }
            if( ImGui::MenuItem( "Copy", "CTRL+C" ) )
            {
            }
            if( ImGui::MenuItem( "Paste", "CTRL+V" ) )
            {
            }
            ImGui::EndMenu();
        }
        if( ImGui::BeginMenu( "View" ) )
        {
            if( ImGui::MenuItem( "Something" ) )
            {
            }
            ImGui::Separator();
            if( ImGui::MenuItem( "Fullscreen", "CTRL+SHIFT+F" ) )
            {
            }
            ImGui::EndMenu();
        }
        if( ImGui::BeginMenu( "Help" ) )
        {
            if( ImGui::MenuItem( "Keybindings", "F1" ) )
            {
            }
            if( ImGui::MenuItem( "About" ) )
            {
            }
            ImGui::EndMenu();
        }

        auto io            = ImGui::GetIO();
        auto & style       = ImGui::GetStyle();
        float font_size_px = 14;
        float right_edge   = ImGui::GetWindowContentRegionMax().x;
        float bar_height   = ImGui::GetWindowContentRegionMax().y + 2 * style.FramePadding.y;
        float width;

        width = 2.5 * font_size_px;
        ImGui::SameLine( right_edge - width );
        if( dark_mode )
        {
            if( ImGui::Button( ICON_FA_SUN, ImVec2( width, bar_height ) ) )
            {
                ImGui::StyleColorsLight();
                clear_color = ImVec4( 0.7f, 0.7f, 0.7f, 1.f );
                dark_mode   = false;
            }
        }
        else
        {
            if( ImGui::Button( ICON_FA_MOON, ImVec2( width, bar_height ) ) )
            {
                apply_charcoal_style();
                clear_color = ImVec4( 0.4f, 0.4f, 0.4f, 1.f );
                dark_mode   = true;
            }
        }
        right_edge -= ( width + style.FramePadding.x );

        std::string label;
        static int N = 4;
        for( int n = N; n > 0; n-- )
        {
            label = fmt::format( "Mode {}", n );
            width = label.length() * font_size_px;
            ImGui::SameLine( right_edge - width );
            if( ImGui::Selectable( label.c_str(), selected_mode == n, 0, ImVec2( width, bar_height ) ) )
                selected_mode = n;
            right_edge -= ( width + style.FramePadding.x );
        }

        ImGui::EndMainMenuBar();
    }
    ImGui::PopFont();
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.txt)
static void help_marker( const char * description )
{
    ImGui::TextDisabled( "(?)" );
    if( ImGui::IsItemHovered() )
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
        ImGui::TextUnformatted( description );
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void show_overlay( bool * p_open )
{
    const float DISTANCE = 50.0f;
    static int corner    = 0;
    ImGuiIO & io         = ImGui::GetIO();
    if( corner != -1 )
    {
        ImVec2 window_pos = ImVec2(
            ( corner & 1 ) ? io.DisplaySize.x - DISTANCE : DISTANCE,
            ( corner & 2 ) ? io.DisplaySize.y - DISTANCE : DISTANCE );
        ImVec2 window_pos_pivot = ImVec2( ( corner & 1 ) ? 1.0f : 0.0f, ( corner & 2 ) ? 1.0f : 0.0f );
        ImGui::SetNextWindowPos( window_pos, ImGuiCond_Always, window_pos_pivot );
    }
    ImGui::SetNextWindowBgAlpha( 0.35f ); // Transparent background
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
                                    | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
                                    | ImGuiWindowFlags_NoNav;
    if( corner != -1 )
        window_flags |= ImGuiWindowFlags_NoMove;
    if( ImGui::Begin( "Example: Simple overlay", p_open, window_flags ) )
    {
        ImGui::Text( "Simple overlay\n"
                     "in the corner of the screen.\n"
                     "(right-click to change position)" );
        ImGui::Separator();
        if( ImGui::IsMousePosValid() )
            ImGui::Text( "Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y );
        else
            ImGui::Text( "Mouse Position: <invalid>" );
        if( ImGui::BeginPopupContextWindow() )
        {
            if( ImGui::MenuItem( "Custom", NULL, corner == -1 ) )
                corner = -1;
            if( ImGui::MenuItem( "Top-left", NULL, corner == 0 ) )
                corner = 0;
            if( ImGui::MenuItem( "Top-right", NULL, corner == 1 ) )
                corner = 1;
            if( ImGui::MenuItem( "Bottom-left", NULL, corner == 2 ) )
                corner = 2;
            if( ImGui::MenuItem( "Bottom-right", NULL, corner == 3 ) )
                corner = 3;
            if( p_open && ImGui::MenuItem( "Close" ) )
                *p_open = false;
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void loop()
try
{
    int display_w, display_h;
#ifdef __EMSCRIPTEN__
    display_w = js_canvas_get_width();
    display_h = js_canvas_get_height();
    glfwSetWindowSize( g_window, display_w, display_h );
#else
    glfwGetFramebufferSize( g_window, &display_w, &display_h );
#endif

#ifdef __EMSCRIPTEN__
    glfwSetWindowSize( g_window, width, height );
    emscripten_webgl_make_context_current( context_imgui );
    glViewport( 0, 0, display_w, display_h );
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#endif

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::PushFont( font_14 );

    show_menu_bar( g_window );
    bool p_open = true;
    show_overlay( &p_open );

    if( show_another_window )
    {
        ImGui::Begin( "Another Window", &show_another_window );
        ImGui::Text( "Hello from another window!" );
        if( ImGui::Button( "Close Me" ) )
            show_another_window = false;
        ImGui::End();
    }

    if( show_demo_window )
    {
        ImGui::SetNextWindowPos( ImVec2( 650, 20 ), ImGuiCond_FirstUseEver );
        ImGui::ShowDemoWindow( &show_demo_window );
    }

    {
        static float f     = 0.0f;
        static int counter = 0;
        ImGui::Text( "Hello, world!" );
        ImGui::SliderFloat( "float", &f, 0.0f, 1.0f );
        ImGui::ColorEdit3( "clear color", (float *)&clear_color );

        ImGui::Text( "Windows" );
        ImGui::Checkbox( "Demo Window", &show_demo_window );
        ImGui::Checkbox( "Another Window", &show_another_window );

        ImGui::Dummy( { 0, 10 } );

        if( ImGui::Button( "notification" ) )
            notification( "This is a notification message" );

        ImGui::Dummy( { 0, 10 } );

        if( ImGui::Button( "Button" ) )
            counter++;
        ImGui::SameLine();
        ImGui::Text( "counter = %d", counter );

        ImGui::Text(
            "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate );
    }

    ImGui::PopFont();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

    glfwSwapBuffers( g_window );

#ifdef __EMSCRIPTEN__
    emscripten_webgl_make_context_current( context_render );
#endif
    draw_gl( display_w, display_h );
}
catch( const std::exception & e )
{
    fmt::print( "Caught std::exception in draw loop! Message: {}\n", e.what() );
}
catch( ... )
{
    fmt::print( "Caught unknown exception in draw loop!\n" );
}

int init()
{
    glfwSetErrorCallback( glfw_error_callback );

    if( !glfwInit() )
    {
        fmt::print( "Failed to initialize GLFW\n" );
        return 1;
    }

    glfwWindowHint( GLFW_SAMPLES, 4 ); // 4x antialiasing
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE ); // We don't want the old OpenGL
    // glfwWindowHint( GLFW_DECORATED, false );
    // glfwWindowHint( GLFW_RESIZABLE, true );
#if __APPLE__
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
#endif

    // Open a window and create its OpenGL context
    int canvasWidth  = 1280;
    int canvasHeight = 720;
    g_window         = glfwCreateWindow( canvasWidth, canvasHeight, "WebGui Demo", NULL, NULL );
    glfwMakeContextCurrent( g_window );
#ifndef __EMSCRIPTEN__
    // Enable vsync
    glfwSwapInterval( 1 );
#endif

    if( g_window == NULL )
    {
        fmt::print( "Failed to open GLFW window.\n" );
        glfwTerminate();
        return -1;
    }

    // Initialize GLAD
    gladLoadGL( (GLADloadfunc)glfwGetProcAddress );

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io   = ImGui::GetIO();
    io.IniFilename = "imgui_state.ini";

    ImGui_ImplGlfw_InitForOpenGL( g_window, false );
    ImGui_ImplOpenGL3_Init();

#ifdef __EMSCRIPTEN__
    EmscriptenWebGLContextAttributes attrs_imgui;
    emscripten_webgl_init_context_attributes( &attrs_imgui );
    attrs_imgui.majorVersion = 1;
    attrs_imgui.minorVersion = 0;
    // attrs_imgui.alpha        = 1;

    context_imgui = emscripten_webgl_create_context( "#imgui-canvas", &attrs_imgui );
    if( context_imgui <= 0 )
    {
        fmt::print( "ImGui WebGL context could not be created! Got {}\n", context_imgui );
        context_imgui = emscripten_webgl_get_current_context();
    }
    if( context_imgui <= 0 )
    {
        fmt::print( "ImGui WebGL context not available! Got {}\n", context_imgui );
    }
    fmt::print( "ImGui WebGL context: {}\n", context_imgui );

    EmscriptenWebGLContextAttributes attrs_render;
    emscripten_webgl_init_context_attributes( &attrs_render );
    attrs_render.majorVersion = 1;
    attrs_render.minorVersion = 0;

    context_render = emscripten_webgl_create_context( "#render-canvas", &attrs_render );
    if( context_render <= 0 )
        fmt::print( "Render WebGL context could not be created! Got {}\n", context_render );
    fmt::print( "Render WebGL context: {}\n", context_render );

    emscripten_webgl_make_context_current( context_imgui );
#endif
    fmt::print( "OpenGL Version: {}\n", glGetString( GL_VERSION ) );

    // Setup style
    // ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();
    apply_charcoal_style();

    // Load Fonts
    font_14 = fonts::karla( 14 );
    font_16 = fonts::karla( 16 );
    font_18 = fonts::karla( 18 );

    // Cursor callbacks
    glfwSetMouseButtonCallback( g_window, ImGui_ImplGlfw_MouseButtonCallback );
    glfwSetScrollCallback( g_window, ImGui_ImplGlfw_ScrollCallback );
    glfwSetKeyCallback( g_window, ImGui_ImplGlfw_KeyCallback );
    glfwSetCharCallback( g_window, ImGui_ImplGlfw_CharCallback );

#ifdef __EMSCRIPTEN__
    js_resize_canvas();
#endif

    return 0;
}

void quit()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow( g_window );
    glfwTerminate();
}

int main()
{
    if( init() != 0 )
        return 1;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop( loop, 0, true );
#else
    while( !glfwWindowShouldClose( g_window ) )
    {
        loop();
    }
#endif

    quit();

    return 0;
}