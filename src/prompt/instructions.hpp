#pragma once

#include "params.hpp"

#include <string>

namespace prompt {
std::string load_instructions(const params::CommandLineParameters &params);
}
