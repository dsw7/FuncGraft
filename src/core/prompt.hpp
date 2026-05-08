#pragma once

#include "configs.hpp"

#include <string>

namespace core {
namespace prompt {
std::string build_prompt(
    const Configurations &configs, const std::string &instructions, const std::string &text_to_edit);
}
} // namespace core
