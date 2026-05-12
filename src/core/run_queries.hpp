#pragma once

#include "configs.hpp"
#include "query_edit_code.hpp"

#include <string>

namespace core {
namespace threading {
queries::OpenAIEdit run_openai_query(
    const Configurations &configs, const std::string &instructions,
    const std::string &code, const std::string &language);
queries::OllamaEdit run_ollama_query(
    const Configurations &configs, const std::string &instructions,
    const std::string &code, const std::string &language);
} // namespace threading
} // namespace core
