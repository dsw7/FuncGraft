#pragma once

#include "structured_output.hpp"

#include <string>

namespace completion {
LLMResponse run_openai_query(const std::string &prompt);
} // namespace completion
