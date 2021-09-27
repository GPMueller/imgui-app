#include <fonts.hpp>
#include <fonts/font_cousine_regular.hpp>
#include <fonts/font_fontawesome.hpp>
#include <fonts/font_karla_regular.hpp>
#include <fonts/font_mono.hpp>

#include <imgui/imgui.h>

namespace fonts
{

ImFont * imgui_default( float size_px )
{
    ImFontConfig default_font_cfg = ImFontConfig();
    default_font_cfg.SizePixels   = size_px;

    ImGuiIO & io = ImGui::GetIO();
    return io.Fonts->AddFontDefault( &default_font_cfg );
}

ImFont * cousine( float size_px )
{
    constexpr int OVERSAMPLE = 2;

    constexpr ImWchar COUSINE_RANGES[] = { 0x0020, 0x00FF, 0x0100, 0x017F, 0 };
    constexpr ImWchar ICONS_RANGES[]   = { 0xf000, 0xf999, 0 };

    ImFontConfig config_letters;
    config_letters.OversampleV = OVERSAMPLE;
    config_letters.OversampleH = OVERSAMPLE;

    ImFontConfig config_glyphs;
    config_glyphs.MergeMode   = true;
    config_glyphs.OversampleV = OVERSAMPLE;
    config_glyphs.OversampleH = OVERSAMPLE;

    ImGuiIO & io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryCompressedTTF(
        cousine_regular_compressed_data, cousine_regular_compressed_size, size_px + 1, &config_letters,
        COUSINE_RANGES );
    auto font = io.Fonts->AddFontFromMemoryCompressedTTF(
        font_awesome_compressed_data, font_awesome_compressed_size, size_px, &config_glyphs, ICONS_RANGES );

    return font;
}

ImFont * karla( float size_px )
{
    constexpr int OVERSAMPLE = 2;

    constexpr ImWchar ICONS_RANGES[] = { 0xf000, 0xf999, 0 };

    float size_karla = size_px + 2;
    float size_icons = size_px;

    if( size_px >= 18 )
    {
        size_karla = size_px + 3;
        size_icons = size_px + 2;
    }

    ImFontConfig config_letters;
    config_letters.OversampleV = OVERSAMPLE;
    config_letters.OversampleH = OVERSAMPLE;

    ImFontConfig config_glyphs;
    config_glyphs.MergeMode   = true;
    config_glyphs.OversampleV = OVERSAMPLE;
    config_glyphs.OversampleH = OVERSAMPLE;

    ImGuiIO & io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryCompressedTTF(
        karla_regular_compressed_data, karla_regular_compressed_size, size_karla, &config_letters );
    auto font = io.Fonts->AddFontFromMemoryCompressedTTF(
        font_awesome_compressed_data, font_awesome_compressed_size, size_icons, &config_glyphs, ICONS_RANGES );

    return font;
}

ImFont * mono( float size_px )
{
    constexpr int OVERSAMPLE = 2;

    constexpr ImWchar ICONS_RANGES[] = { 0xf000, 0xf999, 0 };

    ImFontConfig config_glyphs;
    config_glyphs.MergeMode   = true;
    config_glyphs.OversampleV = OVERSAMPLE;
    config_glyphs.OversampleH = OVERSAMPLE;

    ImGuiIO & io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryCompressedTTF( monospace_compressed_data, monospace_compressed_size, size_px + 1 );
    auto font = io.Fonts->AddFontFromMemoryCompressedTTF(
        font_awesome_compressed_data, font_awesome_compressed_size, size_px, &config_glyphs, ICONS_RANGES );

    return font;
}

} // namespace fonts