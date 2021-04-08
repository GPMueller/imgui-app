#pragma once
#ifndef IMGUI_APP_VERSION_HPP
#define IMGUI_APP_VERSION_HPP

#include <string_view>

namespace version
{

constexpr int major();
constexpr int minor();
constexpr int patch();

const std::string_view version();
const std::string_view revision();
const std::string_view full();

// The latest git tag
const std::string_view latest_tag();

const std::string_view compiler();
const std::string_view compiler_version();
const std::string_view compiler_full();

} // namespace version

#endif