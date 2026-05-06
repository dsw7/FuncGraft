#pragma once

#include "configs.hpp"

#include <string>

namespace core {
namespace prompt {
std::string build_prompt(const Configurations &configs, const std::string &input_text, const std::string &extension);
}
} // namespace core
