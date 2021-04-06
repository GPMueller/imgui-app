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

EM_JS( void, js_resize_canvas, (), { resizeCanvas(); } );
EM_JS( void, js_notify, ( const char * message, int length ), { notifyMessage( UTF8ToString( message, length ) ); } );

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_imgui;
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_render;

void emscripten_app_draw()
{
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

static void framebuffer_size_callback( GLFWwindow * window, int width, int height )
{
    global_app_handle->resize( width, height );
    global_app_handle->draw();
}

namespace ui
{

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

        auto & style     = ImGui::GetStyle();
        float right_edge = ImGui::GetWindowContentRegionMax().x;
        ImGui::PushStyleVar( ImGuiStyleVar_SelectableTextAlign, ImVec2( .5f, .5f ) );

        ImVec2 text_size = ImGui::CalcTextSize( ICON_FA_SUN, NULL, true );
        float width      = 2.5 * font_14->FontSize;
        float height     = text_size.y + 2 * style.ItemInnerSpacing.y;
        ImGui::SetCursorPos( { right_edge - width + style.FramePadding.x, style.FramePadding.y } );
        if( dark_mode )
        {
            if( ImGui::Button( ICON_FA_SUN, ImVec2( width, height ) ) )
            {
                styles::apply_light();
                opengl_renderer.background_color[0] = 0.7f;
                opengl_renderer.background_color[1] = 0.7f;
                opengl_renderer.background_color[2] = 0.7f;
                opengl_renderer.background_color[3] = 1.0f;

                dark_mode = false;
            }
        }
        else
        {
            if( ImGui::Button( ICON_FA_MOON, ImVec2( width, height ) ) )
            {
                styles::apply_charcoal();
                opengl_renderer.background_color[0] = 0.4f;
                opengl_renderer.background_color[1] = 0.4f;
                opengl_renderer.background_color[2] = 0.4f;
                opengl_renderer.background_color[3] = 1.0f;

                dark_mode = true;
            }
        }

        right_edge -= ( width + 4 * style.ItemSpacing.x );

        static int N = 4;
        for( int n = N; n > 0; n-- )
        {
            std::string label = fmt::format( "Mode {}", n );
            text_size         = ImGui::CalcTextSize( label.c_str(), NULL, true );
            width             = text_size.x + 2 * style.ItemInnerSpacing.x;
            height            = text_size.y + 1 * style.ItemInnerSpacing.y;

            ImGui::SameLine( right_edge - width, 0 );
            if( ImGui::Selectable( label.c_str(), selected_mode == n, 0, ImVec2( width, height ) ) )
                selected_mode = n;
            right_edge -= ( width + 2 * style.ItemInnerSpacing.x );
        }

        ImGui::PopStyleVar(); // ImGuiStyleVar_SelectableTextAlign

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
    this->glfw_window.update();

    // --------------- ImGui+GLFW+GL UI context setup
#ifdef __EMSCRIPTEN__
    // Canvas resizes don't trigger the framebuffer resize callback
    const auto & [w, h] = glfw_window.get_size();
    emscripten_webgl_make_context_current( context_imgui );
    glViewport( 0, 0, w, h );
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#endif

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // --------------- ImGui actual UI calls
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
        ImGui::SliderFloat( "##gradient-slider", &opengl_renderer.gradient, 0.0f, 2.0f );

        ImGui::TextUnformatted( "Background colour" );
        ImGui::SameLine();
        ImGui::ColorEdit3( "##background-color-edit", opengl_renderer.background_color );

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

    // --------------- ImGui+GLFW+GL UI drawing
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
    glfwSwapBuffers( glfw_window.glfw_window_handle );

    // --------------- GL render context setup and drawing
#ifdef __EMSCRIPTEN__
    emscripten_webgl_make_context_current( context_render );
#endif
    opengl_renderer.draw();
}

void Application::resize( int width, int height )
{
    glfw_window.resize( width, height );
    const auto & [w, h] = glfw_window.get_size();
    opengl_renderer.set_framebuffer_size( w, h );
}

void Application::run()
{
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop( emscripten_app_draw, 0, true );
#else
    while( !this->glfw_window.should_close() )
    {
        this->draw();
    }
#endif
}

Application::Application( const std::string & title ) : glfw_window( title )
{
    global_app_handle = this;

    // Initialize GLAD
    gladLoadGL( (GLADloadfunc)glfwGetProcAddress );

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io   = ImGui::GetIO();
    io.IniFilename = "imgui_state.ini";

    ImGui_ImplGlfw_InitForOpenGL( glfw_window.glfw_window_handle, false );
    ImGui_ImplOpenGL3_Init();

    // Emscripten context initialization
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
    glfwSetMouseButtonCallback( glfw_window.glfw_window_handle, ImGui_ImplGlfw_MouseButtonCallback );
    glfwSetScrollCallback( glfw_window.glfw_window_handle, ImGui_ImplGlfw_ScrollCallback );
    glfwSetKeyCallback( glfw_window.glfw_window_handle, ImGui_ImplGlfw_KeyCallback );
    glfwSetCharCallback( glfw_window.glfw_window_handle, ImGui_ImplGlfw_CharCallback );
    glfwSetFramebufferSizeCallback( glfw_window.glfw_window_handle, framebuffer_size_callback );

#ifdef __EMSCRIPTEN__
    js_resize_canvas();
#else
    glfw_window.update();
    const auto & [w, h] = glfw_window.get_size();
    this->resize( w, h );
#endif
}

Application::~Application()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

} // namespace ui