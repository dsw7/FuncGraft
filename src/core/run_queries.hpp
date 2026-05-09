#pragma once

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "configs.hpp"

#include <string>

namespace core {
namespace threading {
adapters::OpenAIClassificationResponse classify_instructions_openai(const Configurations &configs, const std::string &prompt);
adapters::OllamaClassificationResponse classify_instructions_ollama(const Configurations &configs, const std::string &prompt);

adapters::OpenAIEditResponse run_openai_query(const Configurations &configs, const std::string &prompt);
adapters::OllamaEditResponse run_ollama_query(const Configurations &configs, const std::string &prompt);
} // namespace threading
} // namespace core
