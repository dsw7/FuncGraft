#pragma once

#include "configs.hpp"

#include <string>

namespace core {
namespace user_prompts {
std::string prompt_check_instructions(const std::string &instructions);
std::string prompt_edit_code(
    const Configurations &configs, const std::string &instructions, const std::string &text_to_edit);
} // namespace user_prompts
} // namespace core
