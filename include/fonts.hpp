#pragma once
#ifndef IMGUI_APP_FONTS_FONTS_HPP
#define IMGUI_APP_FONTS_FONTS_HPP

#include <fonts/fontawesome5_icons.h>

#include <imgui/imgui.h>

namespace fonts
{

ImFont * imgui_default( float size_px );
ImFont * cousine( float size_px );
ImFont * karla( float size_px );
ImFont * mono( float size_px );

} // namespace fonts

#endif