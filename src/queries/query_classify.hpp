#pragma once

#include "adapter_ollama.hpp"
#include "adapter_openai.hpp"
#include "configs.hpp"
#include "responses_ollama.hpp"
#include "responses_openai.hpp"

#include <expected>
#include <string>

namespace queries {

struct OpenAIClassifier: public adapters::OpenAI {
    OpenAIClassifier(const Configurations &configs) :
        adapters::OpenAI(configs) {}

    std::expected<adapters::OpenAIClassification, adapters::OpenAIError> classify_instructions(const std::string &prompt);
};

struct OllamaClassifier: public adapters::Ollama {
    OllamaClassifier(const Configurations &configs) :
        adapters::Ollama(configs) {}

    std::expected<adapters::OllamaClassification, adapters::OllamaError> classify_instructions(const std::string &prompt);
};

} // namespace queries
