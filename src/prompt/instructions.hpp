#pragma once

#include "configs.hpp"

#include <string>

namespace prompt {
std::string load_instructions(const Configurations &params);
}
