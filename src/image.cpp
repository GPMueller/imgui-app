#include <glad/glad.h>

#include <image.hpp>

#include <cmrc/cmrc.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize.h>

#include <fmt/format.h>

#include <array>
#include <memory>

CMRC_DECLARE( resources );

namespace images
{

Image::Image( const std::string_view file_name ) : file_name( file_name )
{
    auto fs   = cmrc::resources::get_filesystem();
    auto file = fs.open( this->file_name );

    file_data  = std::string( file.cbegin(), file.cend() );
    image_data = stbi_load_from_memory(
        reinterpret_cast<const unsigned char *>( file_data.c_str() ),
        int( file_data.size() ) * sizeof( char ) / sizeof( unsigned char ), &width, &height, NULL, 4 );

    if( !image_data )
    {
        fmt::print( "Image: could not load image data from \"{}\"\n", file_name );
    }
}

Image::~Image()
{
    stbi_image_free( image_data );
}

bool Image::get_gl_texture( unsigned int & out_texture )
{
    if( !image_data )
    {
        fmt::print( "Cannot get gl texture without image data\n" );
        return false;
    }

    GLuint image_texture;
    glGenTextures( 1, &image_texture );
    glBindTexture( GL_TEXTURE_2D, image_texture );

    // Setup filtering parameters for display
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // Upload pixels into texture
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data );
    out_texture = image_texture;

    return true;
}

} // namespace images