#include <opengl_renderer.hpp>

#ifndef __EMSCRIPTEN__
#include <glad/glad.h>
#else
#include <GLES3/gl3.h>
#endif

#include <fmt/format.h>

#include <string>

// clang-format off
#ifdef __EMSCRIPTEN__
    #define VERT_SHADER_HEADER "#version 100\n#define in attribute\n#define out varying\nprecision mediump float;\n"
    #define FRAG_SHADER_HEADER "#version 100\n#define in varying\n#define fo_FragColor gl_FragColor\nprecision mediump float;\n"
#else
    #define VERT_SHADER_HEADER "#version 330\n"
    #define FRAG_SHADER_HEADER "#version 330\nout vec4 fo_FragColor;\n"
#endif
// clang-format on

namespace ui
{

const std::string get_gl_error_str( GLenum err )
{
    switch( err )
    {
        case GL_NO_ERROR: return "No error";
        case GL_INVALID_ENUM: return "Invalid enum";
        case GL_INVALID_VALUE: return "Invalid value";
        case GL_INVALID_OPERATION: return "Invalid operation";
        case GL_OUT_OF_MEMORY: return "Out of memory";
#ifndef __EMSCRIPTEN__
        case GL_STACK_OVERFLOW: return "Stack overflow";
        case GL_STACK_UNDERFLOW: return "Stack underflow";
#endif
        default: return "Unknown error";
    }
}

bool check_shader_compilation( GLuint shader )
{
    GLint successful = GL_FALSE;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &successful );
    if( successful == GL_FALSE )
    {
        GLint message_length = 0;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &message_length );
        std::string message( message_length, ' ' );
        glGetShaderInfoLog( shader, message_length, &message_length, message.data() );

        fmt::print( "GL shader compilation error: {}\n", message );
        return false;
    }
    return true;
}

bool check_shader_program( GLuint shader_program )
{
    GLint successful = GL_FALSE;
    glGetProgramiv( shader_program, GL_LINK_STATUS, &successful );
    if( successful == GL_FALSE )
    {
        GLint message_length = 0;
        glGetProgramiv( shader_program, GL_INFO_LOG_LENGTH, &message_length );
        std::string message( message_length, ' ' );
        glGetProgramInfoLog( shader_program, message_length, &message_length, message.data() );

        fmt::print( "GL shader program link error: {}\n", message );
        return false;
    }
    return true;
}

void gl_check_error( const std::string & prefix )
{
    GLenum err = glGetError();
    if( err != GL_NO_ERROR )
        fmt::print( "{}{}\n", prefix, get_gl_error_str( err ) );
}

void OpenglRenderer::initialize_gl()
{
    if( !gl_initialized_ )
    {
        // Shader sources
        const GLchar * vertex_source = VERT_SHADER_HEADER "attribute vec4 position;                 \n"
                                                          "void main()                              \n"
                                                          "{                                        \n"
                                                          "  gl_Position = vec4(position.xyz, 1.0); \n"
                                                          "}                                        \n";
        const GLchar * fragment_source = FRAG_SHADER_HEADER "uniform vec2 resolution;                              \n"
                                                            "uniform float gradient;                               \n"
                                                            "void main()                                           \n"
                                                            "{                                                     \n"
                                                            "  vec2 p = gradient *gl_FragCoord.xy / resolution.xy; \n"
                                                            "  gl_FragColor = vec4(p.x,p.y,0.912,1.0);             \n"
                                                            "}                                                     \n";

        // Create and bind vertex array object
        GLuint vao = 0;
        glGenVertexArrays( 1, &vao );
        glBindVertexArray( vao );

        // Create a vertex buffer object and copy the vertex data to it
        GLuint vbo;
        glGenBuffers( 1, &vbo );
        GLfloat vertices[] = { 0.0f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f };
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
        glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

        // Create and compile the vertex shader
        GLuint vertex_shader = glCreateShader( GL_VERTEX_SHADER );
        glShaderSource( vertex_shader, 1, &vertex_source, nullptr );
        glCompileShader( vertex_shader );
        bool vertex_shader_compiled = check_shader_compilation( vertex_shader );

        // Create and compile the fragment shader
        GLuint fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
        glShaderSource( fragment_shader, 1, &fragment_source, nullptr );
        glCompileShader( fragment_shader );
        bool fragment_shader_compiled = check_shader_compilation( fragment_shader );

        // Link the vertex and fragment shader into a shader program
        shader_program_ = glCreateProgram();
        glAttachShader( shader_program_, vertex_shader );
        glAttachShader( shader_program_, fragment_shader );
        glLinkProgram( shader_program_ );
        glUseProgram( shader_program_ );
        bool shader_program_linked = check_shader_program( shader_program_ );

        gl_check_error( "GL init error: " );

        gl_initialized_ = vertex_shader_compiled && fragment_shader_compiled && shader_program_linked;

        if( !gl_initialized_ )
            fmt::print( "Shader initialization failed!\n" );
    }
}

void OpenglRenderer::draw()
{
    // An array of 3 vectors which represents 3 vertices
    static const GLfloat G_VERTEX_BUFFER_DATA[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    initialize_gl();
    gl_check_error( "--- 1 --- GL draw error: " );

    glViewport( 0, 0, framebuffer_w_, framebuffer_h_ );
    glClearColor( background_color[0], background_color[1], background_color[2], background_color[3] );
    glClear( GL_COLOR_BUFFER_BIT );
    gl_check_error( "--- 2 --- GL draw error: " );

    // Specify the layout of the vertex data
    GLint position_attribute = glGetAttribLocation( shader_program_, "position" );
    gl_check_error( "--- 3 --- GL draw error: " );

    glEnableVertexAttribArray( position_attribute );
    gl_check_error( "--- 4 --- GL draw error: " );

    glVertexAttribPointer( position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    gl_check_error( "--- 5 --- GL draw error: " );

    glUseProgram( shader_program_ );
    gl_check_error( "--- 6 --- GL draw error: " );

    glUniform1f( glGetUniformLocation( shader_program_, "gradient" ), gradient );
    glUniform2f( glGetUniformLocation( shader_program_, "resolution" ), framebuffer_w_, framebuffer_h_ );
    gl_check_error( "--- 7 --- GL draw error: " );

    // Draw a triangle from the 3 vertices
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    gl_check_error( "--- 8 --- GL draw error: " );
}

void OpenglRenderer::set_framebuffer_size( int w, int h )
{
    this->framebuffer_h_ = h;
    this->framebuffer_w_ = w;
}

} // namespace ui