#pragma once

#include <string>

namespace core {
namespace prompt {
std::string build_prompt(const std::string &instructions, const std::string &input_text, const std::string &extension);
}
} // namespace core
