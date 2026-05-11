#pragma once

#include "configs.hpp"
#include "query_classify.hpp"
#include "query_edit_code.hpp"

#include <string>

namespace core {
namespace threading {
queries::OpenAIClassification classify_instructions_openai(const Configurations &configs, const std::string &prompt);
queries::OllamaClassification classify_instructions_ollama(const Configurations &configs, const std::string &prompt);

queries::OpenAIEdit run_openai_query(const Configurations &configs, const std::string &prompt);
queries::OllamaEdit run_ollama_query(const Configurations &configs, const std::string &prompt);
} // namespace threading
} // namespace core
