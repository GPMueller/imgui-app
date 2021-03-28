#include <application.hpp>

#include <fmt/format.h>

#include <exception>

int main()
try
{
    ui::Application app( "ImGui App" );

    // Open the application window
    app.run();
}
catch( const std::exception & e )
{
    fmt::print( "Caught std::exception in main! Message: {}\n", e.what() );
    return 1;
}
catch( ... )
{
    fmt::print( "Caught unknown exception in main!\n" );
    return 1;
}