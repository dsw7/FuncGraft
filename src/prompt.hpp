#pragma once

#include <string>

namespace prompt {
std::string build_prompt(const std::string &instructions, const std::string &input_text, const std::string &extension);
}
