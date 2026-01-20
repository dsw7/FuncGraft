#pragma once

#include "structured_output.hpp"

#include <string>

namespace query_llm {
LLMResponse run_ollama_query(const std::string &prompt);
} // namespace query_llm
