#include <opengl_renderer.hpp>

#include <glad/glad.h>

#include <fmt/format.h>

#include <string>

namespace ui
{

void OpenglRenderer::initialize_gl()
{
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
        shader_program = glCreateProgram();
        glAttachShader( shader_program, vertexShader );
        glAttachShader( shader_program, fragmentShader );
        glLinkProgram( shader_program );
        glUseProgram( shader_program );

        // Check the program
        GLint Result = GL_FALSE;
        int InfoLogLength;
        glGetProgramiv( shader_program, GL_LINK_STATUS, &Result );
        glGetProgramiv( shader_program, GL_INFO_LOG_LENGTH, &InfoLogLength );
        if( InfoLogLength > 0 )
        {
            std::string ProgramErrorMessage;
            ProgramErrorMessage.reserve( InfoLogLength + 1 );
            glGetProgramInfoLog( shader_program, InfoLogLength, NULL, ProgramErrorMessage.data() );
            fmt::print( "GL shader program error: {}\n", ProgramErrorMessage );
        }

        gl_initialized = true;
    }
}

void OpenglRenderer::draw()
{
    initialize_gl();

    // An array of 3 vectors which represents 3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    glViewport( 0, 0, framebuffer_w, framebuffer_h );
    glClearColor( background_color[0], background_color[1], background_color[2], background_color[3] );
    glClear( GL_COLOR_BUFFER_BIT );

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation( shader_program, "position" );
    glEnableVertexAttribArray( posAttrib );
    glVertexAttribPointer( posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0 );

    glUseProgram( shader_program );

    glUniform1f( glad_glGetUniformLocation( shader_program, "gradient" ), gradient );
    glUniform2f( glGetUniformLocation( shader_program, "resolution" ), framebuffer_w, framebuffer_h );

    // Draw a triangle from the 3 vertices
    glDrawArrays( GL_TRIANGLES, 0, 3 );
}

void OpenglRenderer::set_framebuffer_size( int w, int h )
{
    this->framebuffer_h = h;
    this->framebuffer_w = w;
}

} // namespace ui