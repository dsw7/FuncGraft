#pragma once

#include "configs.hpp"
#include "query_classify.hpp"
#include "query_edit_code.hpp"

#include <string>

namespace core {
namespace threading {
queries::OpenAIClassification classify_instructions_openai(const Configurations &configs, const std::string &instructions);
queries::OllamaClassification classify_instructions_ollama(const Configurations &configs, const std::string &instructions);

queries::OpenAIEdit run_openai_query(
    const Configurations &configs, const std::string &instructions,
    const std::string &code, const std::string &file_extension);
queries::OllamaEdit run_ollama_query(
    const Configurations &configs, const std::string &instructions,
    const std::string &code, const std::string &file_extension);
} // namespace threading
} // namespace core
