#pragma once

#include "params.hpp"

#include <string>

namespace prompt {
std::string build_prompt(const params::InternalParameters &params);
}
