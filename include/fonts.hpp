#pragma once
#ifndef IMGUI_CMAKE_IMGUI_IMPL_FONTS_HPP
#define IMGUI_CMAKE_IMGUI_IMPL_FONTS_HPP

#include <imgui/imgui.h>

#include "fonts/fontawesome5_icons.h"

namespace fonts
{

ImFont * imgui_default( float size_px );
ImFont * cousine_regular( float size_px );
ImFont * fontawesome_icons( float size_px );
ImFont * font_combined( float size_px );

} // namespace fonts

#endif