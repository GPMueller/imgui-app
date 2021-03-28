#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include <glad/glad.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <application.hpp>
#include <fonts.hpp>
#include <styles.hpp>

#include <GLFW/glfw3.h>

#include <fmt/format.h>

static ui::Application * global_app_handle;

#ifdef __EMSCRIPTEN__

EM_JS( int, js_canvas_get_width, (), { return Module.canvas.width; } );
EM_JS( int, js_canvas_get_height, (), { return Module.canvas.height; } );
EM_JS( void, js_resize_canvas, (), { resizeCanvas(); } );
EM_JS( void, js_notify, ( const char * message, int length ), { notifyMessage( UTF8ToString( message, length ) ); } );

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_imgui;
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_render;

void emscripten_loop()
{
    glfwPollEvents();
    global_app_handle->draw();
}

#endif

static void notification( const std::string & message )
{
#ifdef __EMSCRIPTEN__
    js_notify( message.c_str(), message.size() );
#else
    // TODO
#endif
}

namespace ui
{

void Application::draw_gl()
{
    // An array of 3 vectors which represents 3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
    static GLuint shaderProgram;
    if( !gl_initialized )
    {
        // Shader sources
        const GLchar * vertexSource = "attribute vec4 position;                 \n"
                                      "void main()                              \n"
                                      "{                                        \n"
                                      "  gl_Position = vec4(position.xyz, 1.0); \n"
                                      "}                                        \n";
        const GLchar * fragmentSource = "precision mediump float;                              \n"
                                        "uniform vec2 resolution;                              \n"
                                        "uniform float gradient;                               \n"
                                        "void main()                                           \n"
                                        "{                                                     \n"
                                        "  vec2 p = gradient *gl_FragCoord.xy / resolution.xy; \n"
                                        "  gl_FragColor = vec4(p.x,p.y,0.912,1.0);             \n"
                                        "}                                                     \n";

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

        // Check the program
        GLint Result = GL_FALSE;
        int InfoLogLength;
        glGetProgramiv( shaderProgram, GL_LINK_STATUS, &Result );
        glGetProgramiv( shaderProgram, GL_INFO_LOG_LENGTH, &InfoLogLength );
        if( InfoLogLength > 0 )
        {
            std::string ProgramErrorMessage;
            ProgramErrorMessage.reserve( InfoLogLength + 1 );
            glGetProgramInfoLog( shaderProgram, InfoLogLength, NULL, ProgramErrorMessage.data() );
            fmt::print( "GL shader program error: {}\n", ProgramErrorMessage );
        }

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

    glUniform1f( glad_glGetUniformLocation( shaderProgram, "gradient" ), gradient );
    glUniform2f( glGetUniformLocation( shaderProgram, "resolution" ), display_w, display_h );

    // Draw a triangle from the 3 vertices
    glDrawArrays( GL_TRIANGLES, 0, 3 );
}

void Application::draw_menu_bar()
{
    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 7.f, 7.f ) );
    ImGui::PushFont( font_14 );

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
                styles::apply_light();
                clear_color = ImVec4( 0.7f, 0.7f, 0.7f, 1.f );
                dark_mode   = false;
            }
        }
        else
        {
            if( ImGui::Button( ICON_FA_MOON, ImVec2( width, bar_height ) ) )
            {
                styles::apply_charcoal();
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

void Application::draw_overlay()
{
    ImGui::PushFont( font_cousine );

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
    if( ImGui::Begin( "Simple overlay", &show_overlay, window_flags ) )
    {
        ImGui::Text( "%.1f FPS (avg. %.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate );

        if( ImGui::IsMousePosValid() )
            ImGui::Text( "Mouse Position: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y );
        else
            ImGui::Text( "Mouse Position: <invalid>" );

        ImGui::Separator();

        ImGui::Text( "(right-click to change position)" );

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
            if( ImGui::MenuItem( "Close" ) )
                show_overlay = false;
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    ImGui::PopFont();
}

void Application::draw()
{
#ifdef __EMSCRIPTEN__
    display_w = js_canvas_get_width();
    display_h = js_canvas_get_height();
    glfwSetWindowSize( glfw_window, display_w, display_h );
#else
    glfwGetFramebufferSize( glfw_window, &display_w, &display_h );
#endif

#ifdef __EMSCRIPTEN__
    emscripten_webgl_make_context_current( context_imgui );
    glViewport( 0, 0, display_w, display_h );
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#endif

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::PushFont( font_12 );

    draw_menu_bar();
    draw_overlay();

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

    ImGui::SetNextWindowPos( { 50, 150 }, ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowSizeConstraints( { 400, 100 }, { FLT_MAX, FLT_MAX } );
    if( ImGui::Begin( "Demo" ) )
    {
        static int counter = 0;
        if( ImGui::Button( "Button" ) )
            counter++;
        ImGui::SameLine();
        ImGui::Text( "counter = %d", counter );

        ImGui::Dummy( { 0, 10 } );

        ImGui::TextUnformatted( "Triangle gradient" );
        ImGui::SameLine();
        ImGui::SliderFloat( "##gradient-slider", &gradient, 0.0f, 2.0f );

        ImGui::TextUnformatted( "Background colour" );
        ImGui::SameLine();
        ImGui::ColorEdit3( "##background-color-edit", (float *)&clear_color );

        ImGui::Dummy( { 0, 10 } );

        ImGui::Text( "Windows" );
        ImGui::Checkbox( "Demo Window", &show_demo_window );
        ImGui::Checkbox( "Another Window", &show_another_window );

        ImGui::Dummy( { 0, 10 } );

        if( ImGui::Button( "notification" ) )
            notification( "This is a notification message" );

        ImGui::End();
    }

    ImGui::PopFont();

    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

    glfwSwapBuffers( glfw_window );

#ifdef __EMSCRIPTEN__
    emscripten_webgl_make_context_current( context_render );
#endif
    draw_gl();
}

void Application::run()
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop( emscripten_loop, 0, true );
#else
    while( !glfwWindowShouldClose( glfw_window ) )
    {
        glfwPollEvents();
        this->draw();
    }
#endif
}

Application::Application( const std::string & title ) : GlfwWindow( title )
{
    global_app_handle = this;

    // ----------------------------- OPENGL
    // Initialize GLAD
    gladLoadGL( (GLADloadfunc)glfwGetProcAddress );

    // ----------------------------- IMGUI
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io   = ImGui::GetIO();
    io.IniFilename = "imgui_state.ini";

    ImGui_ImplGlfw_InitForOpenGL( glfw_window, false );
    ImGui_ImplOpenGL3_Init();

    // ----------------------------- WEBGL
#ifdef __EMSCRIPTEN__
    EmscriptenWebGLContextAttributes attrs_imgui;
    emscripten_webgl_init_context_attributes( &attrs_imgui );
    attrs_imgui.majorVersion = 1;
    attrs_imgui.minorVersion = 0;

    context_imgui = emscripten_webgl_create_context( "#imgui-canvas", &attrs_imgui );
    if( context_imgui <= 0 )
    {
        fmt::print( "ImGui WebGL context could not be created! Got {}\n", context_imgui );
        context_imgui = emscripten_webgl_get_current_context();
    }
    if( context_imgui <= 0 )
        fmt::print( "ImGui WebGL context not available! Got {}\n", context_imgui );
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
    if( dark_mode )
        styles::apply_charcoal();
    else
        styles::apply_light();

    // Load Fonts
    font_cousine = fonts::cousine( 12 );
    font_12      = fonts::karla( 12 );
    font_14      = fonts::karla( 14 );
    font_16      = fonts::karla( 16 );
    font_18      = fonts::karla( 18 );

    // Cursor callbacks
    glfwSetMouseButtonCallback( glfw_window, ImGui_ImplGlfw_MouseButtonCallback );
    glfwSetScrollCallback( glfw_window, ImGui_ImplGlfw_ScrollCallback );
    glfwSetKeyCallback( glfw_window, ImGui_ImplGlfw_KeyCallback );
    glfwSetCharCallback( glfw_window, ImGui_ImplGlfw_CharCallback );

#ifdef __EMSCRIPTEN__
    js_resize_canvas();
#endif
}

Application::~Application()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

} // namespace ui