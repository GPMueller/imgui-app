#pragma once
#ifndef IMGUI_APP_FONTS_STYLES_HPP
#define IMGUI_APP_FONTS_STYLES_HPP

#include <imgui/imgui.h>

namespace styles
{

void apply_light( ImGuiStyle * dst = NULL );
void apply_charcoal( ImGuiStyle * dst = NULL );

} // namespace styles

#endif