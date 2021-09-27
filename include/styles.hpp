#pragma once
#ifndef IMGUI_APP_STYLES_HPP
#define IMGUI_APP_STYLES_HPP

#include <imgui/imgui.h>

namespace styles
{

void apply_light( ImGuiStyle * dst = nullptr );
void apply_charcoal( ImGuiStyle * dst = nullptr );

} // namespace styles

#endif