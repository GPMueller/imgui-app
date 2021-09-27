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

    static constexpr ImWchar COUSINE_RANGES[] = { 0x0020, 0x00FF, 0x0100, 0x017F, 0 };
    static constexpr ImWchar ICONS_RANGES[]   = { 0xf000, 0xf999, 0 };

    ImFontConfig config_letters;
    config_letters.OversampleV = OVERSAMPLE;
    config_letters.OversampleH = OVERSAMPLE;

    ImFontConfig config_glyphs;
    config_glyphs.MergeMode   = true;
    config_glyphs.OversampleV = OVERSAMPLE;
    config_glyphs.OversampleH = OVERSAMPLE;

    ImGuiIO & io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryCompressedTTF(
        COUSINE_REGULAR_COMPRESSED_DATA, COUSINE_REGULAR_COMPRESSED_SIZE, size_px + 1, &config_letters,
        COUSINE_RANGES );
    auto font = io.Fonts->AddFontFromMemoryCompressedTTF(
        FONT_AWESOME_COMPRESSED_DATA, FONT_AWESOME_COMPRESSED_SIZE, size_px, &config_glyphs, ICONS_RANGES );

    return font;
}

ImFont * karla( float size_px )
{
    constexpr int OVERSAMPLE = 2;

    static constexpr ImWchar ICONS_RANGES[] = { 0xf000, 0xf999, 0 };

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
        KARLA_REGULAR_COMPRESSED_DATA, KARLA_REGULAR_COMPRESSED_SIZE, size_karla, &config_letters );
    auto font = io.Fonts->AddFontFromMemoryCompressedTTF(
        FONT_AWESOME_COMPRESSED_DATA, FONT_AWESOME_COMPRESSED_SIZE, size_icons, &config_glyphs, ICONS_RANGES );

    return font;
}

ImFont * mono( float size_px )
{
    constexpr int OVERSAMPLE = 2;

    static constexpr ImWchar ICONS_RANGES[] = { 0xf000, 0xf999, 0 };

    ImFontConfig config_glyphs;
    config_glyphs.MergeMode   = true;
    config_glyphs.OversampleV = OVERSAMPLE;
    config_glyphs.OversampleH = OVERSAMPLE;

    ImGuiIO & io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryCompressedTTF( MONOSPACE_COMPRESSED_DATA, MONOSPACE_COMPRESSED_SIZE, size_px + 1 );
    auto font = io.Fonts->AddFontFromMemoryCompressedTTF(
        FONT_AWESOME_COMPRESSED_DATA, FONT_AWESOME_COMPRESSED_SIZE, size_px, &config_glyphs, ICONS_RANGES );

    return font;
}

} // namespace fonts